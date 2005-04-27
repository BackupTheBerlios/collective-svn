#!/usr/bin/perl

######
## Simple XTR .mod to Lightwave .obj converter
######

print "Processing $ARGV[0]\n";

$vertexNdx = 0;
$faceNdx   = 0;

$texCnt = 0;

open(IN,$ARGV[0]);
while($line = <IN>) {
	$line =~ s/[\n\r]+//sg;
	
	if(      $line =~ /^P / ) {  # Vertex
		$line =~ s/^P //;
		@spl = split(/\,\s+/,$line);
		$vertex[$vertexNdx] = [$spl[0],$spl[1],$spl[2]];
		$vertexNdx++;
	} elsif( $line =~ /^F / ) { # Single-sided face
		$line =~ s/^F //;
		@spl = split(/\,\s*/,$line);
		$prefaceNdx = $faceNdx;
		$texCnt = 0;
		$i = 0;
		while($i < ($spl[0]-2)) {
			$face[$faceNdx] = [$spl[1+$i],$spl[2+$i],$spl[3+$i]];
			print "Fid: $face[$faceNdx][0] - $face[$faceNdx][1] - $face[$faceNdx][2]\n";
			$faceNdx++;
			$i++;
		}
	} elsif( $line =~ /^B / ) { # Extruded surface
		$line =~ s/^B //;
		@spl = split(/\,\s*/,$line);
		$prefaceNdx = $faceNdx;
		$texCnt = 0;
		$i = 0;
		while($i < ($spl[0]-2)) {
			$face[$faceNdx] = [$spl[1+$i],$spl[2+$i],$spl[3+$i]];
			$faceNdx++;
			$i++;
		}
	} elsif( $line =~ /^TEX / ) { # Texture coordinate
		$line =~ s/^TEX //;
		@spl = split(/\,\s+/,$line);
		
		$tex[$prefaceNdx][$texCnt] = [$spl[0],$spl[1]];
		#print "texCnt = $texCnt\n";
		$texCnt++;
		if( $texCnt eq 3 ) { $texCnt = 0; $prefaceNdx++;}
	}
}
close(IN);


print "Vertices: " . @vertex . "\n";
print "Face    : " . @face   . "\n";

print "Vertex0 = $vertex[0][0], $vertex[0][1], $vertex[0][2]\n";
print "Vertex1 = $vertex[1][0], $vertex[1][1], $vertex[1][2]\n";

print "Face0 = $face[0][0], $face[0][1], $face[0][2]\n";
print "Tex0  = $tex[0][0][0], $tex[0][0][1]\n";
print "Tex0  = $tex[0][1][0], $tex[0][1][1]\n";
print "Tex0  = $tex[0][2][0], $tex[0][2][1]\n";
print "Face1 = $face[1][0], $face[41][1], $face[41][2]\n";
print "Tex1  = $tex[1][0][0], $tex[1][0][1]\n";
print "Tex1  = $tex[1][1][0], $tex[1][1][1]\n";
print "Tex1  = $tex[1][2][0], $tex[1][2][1]\n";
