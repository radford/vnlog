#!/usr/bin/perl
use strict;
use warnings;

use feature ':5.10';

use FindBin '$Bin';
use lib $Bin;

use TestHelpers qw(test_init check);

use Term::ANSIColor;
my $Nfailed = 0;




my $data1 = <<'EOF';
#!/bin/xxx
## asdf
# a b e
## asdf
1a 22b 9
# asdf
5a 32b 10
## zxcv
6a 42b 11
EOF

my $data2 = <<'EOF';
#!/bin/xxx
## zxcv
# b c d e
## zxcv
22b 1c 5d 8
# asdf
32b 5c 6d 9
## zxcv
52b 6c 7d 10
EOF

my $data3 = <<'EOF';
# b f
22b 18
32b 29
52b 30
62b 11
EOF

my $data_int = <<'EOF';
# a b
1 a
2 b
3 c
EOF

my $data_int_dup = <<'EOF';
# a c c
1 10 A
2 11 B
3 12 C
EOF


test_init('vnl-join', \$Nfailed,
          '$data1'       => $data1,
          '$data2'       => $data2,
          '$data3'       => $data3,
          '$data_int'    => $data_int,
          '$data_int_dup'=> $data_int_dup);





check( 'ERROR', (), '$data1', '$data2' );
check( 'ERROR', qw(-e x), '$data1', '$data2' );
check( 'ERROR', qw(-t x), '$data1', '$data2' );
check( 'ERROR', qw(-1 x), '$data1', '$data2' );
check( 'ERROR', qw(-2 x), '$data1', '$data2' );
check( 'ERROR', qw(-1 x -2 x), '$data1', '$data2' );
check( 'ERROR', qw(-1 b -2 d), '$data1', '$data2' );
check( 'ERROR', qw(-j x), '$data1', '$data2' );
check( 'ERROR', qw(-1 x -j x), '$data1', '$data2' );
check( 'ERROR', qw(-1 b -2 b -j b), '$data1', '$data2' );

check( <<'EOF', qw(-1 b -2 b), '$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
EOF

check( <<'EOF', qw(-1 b -2 b --vnl-prefix1 aaa --vnl-suffix2 bbb), '$data1', '$data2' );
# b aaaa aaae cbbb dbbb ebbb
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
EOF

check( <<'EOF', qw(-1 b -2 b -o), '0,1.a,2.e', qw( --vnl-prefix1 aaa --vnl-suffix2 bbb), '$data1', '$data2' );
# b aaaa ebbb
22b 1a 8
32b 5a 9
EOF

check( <<'EOF', qw(-1b -2b), '$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
EOF

check( <<'EOF', qw(-j b), '$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
EOF

check( <<'EOF', qw(-j b), '$data1', '-$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
EOF

check( <<'EOF', qw(-j b), '-$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
EOF

check( <<'EOF', qw(-j b), '-$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
EOF

check( <<'EOF', qw(-jb -a1), '-$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
42b 6a 11 - - -
EOF

check( <<'EOF', qw(-jb -a2), '-$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
52b - - 6c 7d 10
EOF

check( <<'EOF', qw(-jb -a2 -a1), '$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
42b 6a 11 - - -
52b - - 6c 7d 10
EOF

check( <<'EOF', qw(-jb -a-), '$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
42b 6a 11 - - -
52b - - 6c 7d 10
EOF

check( <<'EOF', qw(-jb -a1 -a2), '$data1', '$data2' );
# b a e c d e
22b 1a 9 1c 5d 8
32b 5a 10 5c 6d 9
42b 6a 11 - - -
52b - - 6c 7d 10
EOF

check( <<'EOF', qw(-jb -v1), '$data1', '$data2' );
# b a e c d e
42b 6a 11 - - -
EOF

check( <<'EOF', qw(-jb -v2), '-$data1', '$data2' );
# b a e c d e
52b - - 6c 7d 10
EOF

check( <<'EOF', qw(-jb -v1 -v2), '$data1', '$data2' );
# b a e c d e
42b 6a 11 - - -
52b - - 6c 7d 10
EOF

check( <<'EOF', qw(-jb -v-), '$data1', '$data2' );
# b a e c d e
42b 6a 11 - - -
52b - - 6c 7d 10
EOF

check( <<'EOF', qw(-jb -o),  '1.a,0,2.d,1.e,0,2.e', '$data1', '$data2' );
# a b d e b e
1a 22b 5d 9 22b 8
5a 32b 6d 10 32b 9
EOF

check( 'ERROR', qw(-jb -o),  '9.a,0,2.d,1.e,0,2.e', '$data1', '$data2' );
check( 'ERROR', qw(-jb -o),  '1.ax,0,2.d,1.e,0,2.e', '$data1', '$data2' );
check( 'ERROR', qw(-jb -o),  '1.a,9,2.d,1.e,0,2.e', '$data1', '$data2' );


# these keys are sorted numerically, not lexicographically
check( 'ERROR', qw(-j e), '$data1', '$data2' );
check( <<'EOF', qw(-j e --vnl-sort - --vnl-suffix1 1), '$data1', '$data2' );
# e a1 b1 b c d
10 5a 32b 52b 6c 7d
9 1a 22b 32b 5c 6d
EOF
check( <<'EOF', qw(-j e --vnl-sort n --vnl-suffix1 1), '$data1', '$data2' );
# e a1 b1 b c d
9 1a 22b 32b 5c 6d
10 5a 32b 52b 6c 7d
EOF

# Now make sure irrelevant dups don't break me
check( <<'EOF', qw(-j a), '$data_int', '$data_int_dup' );
# a b c c
1 a 10 A
2 b 11 B
3 c 12 C
EOF


# But that relevant dups do
check( 'ERROR', qw(-j c), '$data_int', '$data_int_dup' );

# 3-way joins
check( <<'EOF', qw(-jb), '$data1', '$data2', '$data3');
# b a e c d e f
22b 1a 9 1c 5d 8 18
32b 5a 10 5c 6d 9 29
EOF

check( 'ERROR', qw(-1b), '$data1', '$data2', '$data3');

# I check -a- with ALL ordering of passed-in data
check( <<'EOF', qw(-jb -a-), '$data1', '$data2', '$data3');
# b a e c d e f
22b 1a 9 1c 5d 8 18
32b 5a 10 5c 6d 9 29
42b 6a 11 - - - -
52b - - 6c 7d 10 30
62b - - - - - 11
EOF

check( <<'EOF', qw(-jb -a-), '$data1', '$data3', '$data2');
# b a e f c d e
22b 1a 9 18 1c 5d 8
32b 5a 10 29 5c 6d 9
42b 6a 11 - - - -
52b - - 30 6c 7d 10
62b - - 11 - - -
EOF

check( <<'EOF', qw(-jb -a-), '$data2', '$data1', '$data3');
# b c d e a e f
22b 1c 5d 8 1a 9 18
32b 5c 6d 9 5a 10 29
42b - - - 6a 11 -
52b 6c 7d 10 - - 30
62b - - - - - 11
EOF

check( <<'EOF', qw(-jb -a-), '$data2', '$data3', '$data1');
# b c d e f a e
22b 1c 5d 8 18 1a 9
32b 5c 6d 9 29 5a 10
42b - - - - 6a 11
52b 6c 7d 10 30 - -
62b - - - 11 - -
EOF

check( <<'EOF', qw(-jb -a-), '$data3', '$data1', '$data2');
# b f a e c d e
22b 18 1a 9 1c 5d 8
32b 29 5a 10 5c 6d 9
42b - 6a 11 - - -
52b 30 - - 6c 7d 10
62b 11 - - - - -
EOF

check( <<'EOF', qw(-jb -a-), '$data3', '$data2', '$data1');
# b f c d e a e
22b 18 1c 5d 8 1a 9
32b 29 5c 6d 9 5a 10
42b - - - - 6a 11
52b 30 6c 7d 10 - -
62b 11 - - - - -
EOF

# 3-way -o. Generally unsupported
check( 'ERROR', '-jb', '-o', '1.a,0,3.f,2.c,3.b,1.b,1.e,2.e', '$data1', '$data2', '$data3');
check( <<'EOF', qw(-jb -o auto), '$data1', '$data2', '$data3');
# b a e c d e f
22b 1a 9 1c 5d 8 18
32b 5a 10 5c 6d 9 29
EOF




if($Nfailed == 0 )
{
    say colored(["green"], "All tests passed!");
    exit 0;
}
else
{
    say colored(["red"], "$Nfailed tests failed!");
    exit 1;
}
