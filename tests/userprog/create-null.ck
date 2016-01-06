# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF', <<'EOF']);
(create-null) begin
(create-null) create(NULL): 0
(create-null) end
create-null: exit(0)
EOF
(create-null) begin
create-null: exit(-1)
EOF
pass;
