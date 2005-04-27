#!/usr/bin/perl -w
# rename program by Larry Wall
  $op = shift or die "Usage: rename expr [files]\n";
  chomp(@ARGV = <STDIN>) unless @ARGV;
  for (@ARGV) {
     $was = $_;
     eval $op;
     die $@ if $@;
     rename($was,$_) unless $was eq $_;
  }
