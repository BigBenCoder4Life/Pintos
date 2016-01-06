# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF', <<'EOF']);
(create-bad-ptr) begin
(create-bad-ptr) create(0x20101234): 1
(create-bad-ptr) end
create-bad-ptr: exit(0)
EOF
(create-bad-ptr) begin
create-bad-ptr: exit(-1)
EOF
pass;
