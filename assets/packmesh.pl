#!/usr/bin/perl

use strict;
use Data::Dumper;
use Math::Vector::Real;

die "$0 <file>" if !@ARGV;

my $file = shift @ARGV;

my (%materials, @pos, @normals, @vnormals, %verts, @verts, @tris);

# read materials

{
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
		push @vnormals, V(0, 0, 0);
	} elsif (/^vn (\S+) (\S+) (\S+)/) {
		push @normals, V($1, $2, $3);
	} elsif (/^usemtl (.*)/) {
		$current_mtl = $1;
	} elsif (/^f (.*)/) {
		my $r = $1;

		my @v;

		while ($r =~ /(\d*)\/\/(\d*)/g) {
			my ($pos_index, $normal_index) = ($1, $2);

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

		# compute face normal, add to vertex normal

		my $v0 = $pos[$verts[$v[0]]->{pos}];
		my $v1 = $pos[$verts[$v[1]]->{pos}];
		my $v2 = $pos[$verts[$v[2]]->{pos}];

		my $n = ($v1 - $v0)->versor x ($v2 - $v0)->versor;

		$vnormals[$verts[$_]->{pos}] += $n for @v;
	}
}

close OBJ;
}

$_ = $_->versor for @vnormals;

print pack 'C4', ord('M'), ord('E'), ord('S'), ord('H');

print pack 'S', scalar @verts;

for (@verts) {
	my $p = $pos[$_->{pos}];

	print pack 'f3', @{$pos[$_->{pos}]};
	print pack 'f3', @{$normals[$_->{normal}]};
	print pack 'f3', @{$vnormals[$_->{pos}]};
	print pack 'f3', @{$materials{$_->{material}}->{Kd}};
}

print pack 'S', scalar @tris;
print pack 'S3', @{$_->{verts}} for @tris;
