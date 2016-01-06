# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF', <<'EOF']);
null: exit(-1)
EOF
system call!
EOF
pass;
