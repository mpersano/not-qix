#!/usr/bin/perl

use strict;
use Data::Dumper;
use Math::Vector::Real;

my ($compute_normals, $no_material);

while (@ARGV and $ARGV[0] =~ /^--/) {
	if ($ARGV[0] eq '--compute-normals') {
		$compute_normals = 1;
		shift @ARGV;
	} elsif ($ARGV[0] eq '--no-material') {
		$no_material = 1;
		shift @ARGV;
	} else {
		usage();
	}
}

if (!@ARGV) {
	usage();
}

my $file = shift @ARGV;

my (%materials, @pos, @normals, %verts, @verts, @tris);

# read materials

if (!$no_material) {
	open MTL, "$file.mtl" or die "failed to open $file.mtl: $!";

	my $current_mtl;

	while (<MTL>) {
		chomp;

		if (/^newmtl (.*)/) {
			$current_mtl = $materials{$1} = {};
		} elsif (/^Kd (\S+) (\S+) (\S+)/) {
			$current_mtl->{Kd} = [ $1, $2, $3 ];
		}
	}

	close MTL;
}

# read mesh

{
open OBJ, "$file.obj" or die "failed to open $file.obj: $!";

my $current_mtl;

while (<OBJ>) {
	chomp;

	if (/^v (\S+) (\S+) (\S+)/) {
		push @pos, V($1, $2, $3);

		if ($compute_normals) {
			push @normals, V(0, 0, 0);
		}
	} elsif (/^vn (\S+) (\S+) (\S+)/) {
		if (!$compute_normals) {
			push @normals, V($1, $2, $3);
		}
	} elsif (/^usemtl (.*)/) {
		if (!$no_material) {
			$current_mtl = $1;
		}
	} elsif (/^f (.*)/) {
		my $r = $1;

		my @v;

		while ($r =~ /(\d*)\/\/(\d*)/g) {
			my ($pos_index, $normal_index) = ($1, $compute_normals ? $1 : $2);

			my $key = "$pos_index,$normal_index,$current_mtl";

			if (not exists $verts{$key}) {
				push @verts, { pos => $pos_index - 1, normal => $normal_index - 1, material => $current_mtl };
				$verts{$key} = $#verts;
			}

			push @v, $verts{$key};
		}

		for my $i (1 .. scalar @v - 2) {
			my ($v0, $v1, $v2) = ($v[0], $v[$i], $v[$i + 1]);
			push @tris, { verts => [ $v0, $v1, $v2 ] };
		}

		if ($compute_normals) {
			my $v0 = $pos[$verts[$v[0]]->{pos}];
			my $v1 = $pos[$verts[$v[1]]->{pos}];
			my $v2 = $pos[$verts[$v[2]]->{pos}];

			my $n = ($v1 - $v0)->versor x ($v2 - $v0)->versor;

			$normals[$verts[$_]->{pos}] += $n for @v;
		}
	}
}

close OBJ;
}

if ($compute_normals) {
	$_ = $_->versor for @normals;
}

print pack 'C4', ord('M'), ord('E'), ord('S'), ord('H');

print pack 'C', !$no_material;

print pack 'S', scalar @verts;

for (@verts) {
	my $p = $pos[$_->{pos}];

	print pack 'f3', @{$pos[$_->{pos}]};
	print pack 'f3', @{$normals[$_->{normal}]};

	if (!$no_material) {
		print pack 'f3', @{$materials{$_->{material}}->{Kd}};
	}
}

print pack 'S', scalar @tris;
print pack 'S3', @{$_->{verts}} for @tris;

sub usage
{
	die <<EOL;
Usage: $0 [options] file

Options:
  --compute-normals	ignore normals in file and compute from geometry
  --no-material		don't read material, don't output vertex colors
EOL
}
