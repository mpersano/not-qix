#!/usr/bin/perl

use strict;
use Data::Dumper;

my (@pos, @normals, %verts, @verts, @tris);

while (<>) {
	chomp;

	if (/^v (\S+) (\S+) (\S+)/) {
		push @pos, [ $1, $2, $3 ];
	} elsif (/^vn (\S+) (\S+) (\S+)/) {
		push @normals, [ $1, $2, $3 ];
	} elsif (/^f (.*)/) {
		my $r = $1;

		my @v;

		while ($r =~ /(\d*)\/\/(\d*)/g) {
			my ($pos_index, $normal_index) = ($1, $2);

			my $key = "$pos_index,$normal_index";

			if (not exists $verts{$key}) {
				push @verts, { pos => $pos_index - 1, normal => $normal_index - 1 };
				$verts{$key} = $#verts;
			}

			push @v, $verts{$key};
		}

		for my $i (1 .. scalar @v - 2) {
			my ($v0, $v1, $v2) = ($v[0], $v[$i], $v[$i + 1]);
			push @tris, [ $v0, $v1, $v2 ];
		}
	}
}

print pack 'C4', ord('M'), ord('E'), ord('S'), ord('H');

print pack 'S', scalar @verts;

for (@verts) {
	print pack 'f3', @{$pos[$_->{pos}]};
	print pack 'f3', @{$normals[$_->{normal}]};
}

print pack 'S', scalar @tris;
print pack 'S3', @{$_} for @tris;
