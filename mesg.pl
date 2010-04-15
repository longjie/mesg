#!/usr/bin/perl

# --arch システムの名前
# --port port name
# --recv 受信するmenu
# --send 送信するmenu

# 出力するファイル
# mesg.h - メッセージの宣言 (一つ)
# mesg_port.h - port用の関数宣言 (複数)
# mesg_port.c - port用の関数定義 (複数)

use File::Basename;
use Getopt::Long;

# port, archを取得
GetOptions('arch=s' => \$arch, 'port=s' => \$port, 'recv=s' => \@recv, 'send=s' => \@send);

# portの確認
if (not grep $port ("sci", "udp")) {
    die "Invalid port name $port.\n";
}

# archの確認
if (not ($arch eq "win32")) {
    die "Invalid arch name $arch.\n";
}

# sendとrecvの確認
if ((not defined @send) and (not defined @recv)) {
    die "No msg file.\n";
}

for (my $i=0; $i<@send; $i++) {
    my $menu = fileparse ($send[$i], ('.msg'));
    print "send[$i] = $menu\n";
}

for (my $i=0; $i<@recv; $i++) {
    my $menu = fileparse ($recv[$i], ('.msg'));
    print "recv[$i] = $menu\n";
}

# both: 使用するmessage fileの一覧（重複なし)
%send = make_hash (@send);
%recv = make_hash (@recv);
%both = make_hash (@send, @recv);


sub make_hash {
    my %hash;
    for (my $i=0; $i<@_; $i++) {
	my ($name, $path, $suff) = fileparse ($_[$i], ('.msg'));
	$hash{$name} = $_[$i];
    }	
    return %hash;
}

sub dirsparse {
    my @menu;
    for (my $i=0; $i<@_; $i++) {
	my ($name, $path, $suff) = fileparse ($_[$i], ('.msg'));
	push (@menu, $name);
    }	
    return sort @menu;
}

# 重複の無いリストへの変換
sub list_unique {
    my %hash  = ();

    foreach my $value ( @_ ){
        $hash{$value} = 1;
    }
    return (keys %hash);
}

# mesg.hの出力
create_mesg_header_file ($port, $arch, \%both);

# mesg_port.hの出力
create_port_header_file ($port, $arch, \%send, \%recv);

# mesg_port.cの出力
create_port_source_file ($port, $arch, \%send, \%recv);

exit;

# mesg_port_menu.cファイルを生成する
# port: port名
# arch: arch名
# menu: menuファイル名のリスト
sub create_port_source_file
{
    my $file, ($port, $arch, $send, $recv) = @_;

    open $file, ">mesg_${port}.c" or die "Can't open file \"mesg_${port}.c\".";

    # includeを出力
    print $file "#include \"mesg.h\"\n";

    # includeを出力
    for (my $i=0; $i<@menu; $i++) {
	print $file "#include \"mesg_${port}_${arch}_$menu[$i].h\"\n";
    }
    print $file "\n";

    # 送信用関数宣言を出力
    foreach my $menu (sort keys %$send) {
	@node = load_mesg_file ($send{$menu});
	# 送信用関数を出力
	for (my $i=0; $i<@node; $i++) {
	    print_send_func_decl ($file, $port, $arch, $menu, $node[$i]);
	}
    }
    # 受信用関数を出力
    foreach my $menu (sort keys %$recv) {
	my @node = load_mesg_file ($recv{$menu});
	for (my $i=0; $i<@node; $i++) {
	    print_recv_func_decl ($file, $port, $arch, $menu, $node[$i]);
	}
    }
    # mesg_port_menu_numbを出力
    print $file "\n";
    print $file "int mesg_${port}_menu_numb = ", scalar(keys %$recv), ";\n";
    print $file "\n";

    # mesg受信用関数ベクタを出力
    foreach my $menu (sort keys %$recv) {
	my @node = load_mesg_file ($recv{$menu});
	print $file "\n";
	print $file "mesg_${port}_recv_func_t mesg_${port}_recv_${menu}_func_vect[] = {\n";
	for (my $i=0; $i<@node; $i++) {
	    print_recv_func_vect_decl ($file, $port, $arch, $menu, $node[$i]);
	}
	print $file "};\n";
    }
    # menu受信用関数を出力
    foreach my $menu (sort keys %$recv) {
	print_recv_all_func_decl ($file, $port, $arch, $menu);
    }

    # menu受信用関数ベクタを出力
    print $file "mesg_${port}_menu_t mesg_${port}_menu_vect[] = {\n";
    foreach my $menu (sort keys %$recv) {
	print $file "    {\"", uc($menu), "\", mesg_${port}_recv_$menu}, \n";
    }
    print $file "};\n";

}

sub load_mesg_file
{
    my ($file) = @_;
    my @node;

    # 生成ファイルをオープン
    open $file, "<$file" or die "Can't open file \"$file\".";
    while (<$file>) {
	# 改行を削除
	$_ =~ s/\r?\n//;
	# 空白を削除
	$_ =~ s/\s//g;
	# コメントを削除
	$_ =~ s/\#.*$//;
	# 空行は無視
	unless ($_) {
	    next;
	}
	push (@node, $_);
    }
    close $file;
    return @node;
}

# mesg_menu.hファイルを生成する
# file: a list of name of the message menu file.
sub create_mesg_header_file
{
    my $file, ($port, $arch, $both) = @_;

    # ファイルを開く
    open $file, ">mesg.h" or die "Can't open file \"mesg.h\".";

    print $file "#ifndef __MESG_H__\n";
    print $file "#define __MESG_H__\n";
    print $file "\n";

    print $file "#include \"mesg_${port}_${arch}.h\"\n";
    print $file "\n";

    # メッセージIDを出力
    foreach my $menu (sort keys %$both) {
	@node = load_mesg_file ($both{$menu});
	for (my $j=0; $j<@node; $j++) {
	    my ($name, $type, $size) = split_node ($node[$j]);
	    print "$node[$j]\n";
	    print $file "#define MESG_", uc($menu), "_", uc($name), " ($j)\n";
	}
    }
    print $file "\n";

    # extern C
    print $file "#ifdef __cplusplus\n";
    print $file "extern \"C\" {\n";
    print $file "#endif\n\n";

    # 関数の宣言
    # メッセージIDを出力
    foreach  my $menu (sort keys %$both) {
	my @node = load_mesg_file ($both{$menu});
	for (my $j=0; $j<@node; $j++) {
	    print_user_func_defs ($file, $menu, $node[$j]);
	}
    }
    print $file "\n";
    # extern C
    print $file "#ifdef __cplusplus\n";
    print $file "}\n";
    print $file "#endif\n";

    print $file "#endif /* __MESG_H__ */\n";
}

# mesg_port.hファイルを生成する
sub create_port_header_file
{
    my $file, ($port, $arch, $send, $recv) = @_;

    open $file, ">mesg_${port}.h" or die "Can't open file \"mesg_${port}.h\".";

    print $file "#ifndef __MESG_", uc("mesg_${port}_h"), "__\n";
    print $file "#define __MESG_", uc("mesg_${port}_h"), "__\n";
    print $file "\n";
    print $file "#include \"mesg_${port}_${arch}.h\"\n";
    print $file "#include \"mesg.h\"\n";
    print $file "#include \"pack.h\"\n";
    print $file "\n";

    # extern C
    print $file "#ifdef __cplusplus\n";
    print $file "extern \"C\" {\n";
    print $file "#endif\n\n";

    # 送信用関数宣言を出力
    foreach my $menu (sort keys %$send) {
	@node = load_mesg_file ($send{$menu});
	
	for (my $i=0; $i<@node; $i++) {
        print "node[$i]:$node[$i]\n";
	    print_send_func_defs ($file, $menu, $node[$i]);
	}
    }
    # 受信用関数宣言を出力
    foreach my $menu (sort keys %$recv) {
	print_recv_func_defs ($file, $port, $arch, $menu);
    }
    # extern C
    print $file "\n";
    print $file "#ifdef __cplusplus\n";
    print $file "}\n";
    print $file "#endif\n";

    print $file "\n";
    foreach my $menu (sort keys %$recv) {
	print $file "extern mesg_${port}_recv_func_t mesg_${port}_recv_${menu}_func_vect[];\n";
    }
    print $file "\n";
    print $file "#endif /* __MESG_", uc("mesg_${port}_h"), "__ */\n";
}

# ソースファイルを生成する
sub create_port_arch_mesg_source_file
{
    my ($port, $arch, $menu, @node) = @_;

    open $file, ">mesg_${port}_${arch}_${menu}.c" or die "Can't open file \"mesg_${port}_${arch}_$menu.c\".";

    # includeを出力
    print $file "#include \"mesg_${port}_${arch}_${menu}.h\"\n";

    # 送信用関数を出力
    for (my $i=0; $i<@node; $i++) {
        print_send_func_decl ($file, $port, $arch, $menu, $node[$i]);
    }

    # 受信用関数を出力
    for (my $i=0; $i<@node; $i++) {
        print_recv_func_decl ($file, $port, $arch, $menu, $node[$i]);
    }
    # menu受信用関数を出力
    print_recv_all_func_decl ($file, $port, $arch, $menu);

    # 受信用関数ベクタを出力
    print $file "\n";
    print $file "mesg_${port}_recv_func_t mesg_${port}_recv_${menu}_func_vect[] = {\n";
    for (my $i=0; $i<@node; $i++) {
        print_recv_func_vect_decl ($file, $port, $arch, $menu, $node[$i]);
    }
    print $file "};\n";
}

# ユーザ定義関数の宣言の出力
sub print_user_func_defs
{
    my ($file, $menu, $node) = @_;
    my ($name, $type, $size) = split_node ($node);

    print $file "int ${menu}_${name} (mesg_${port}_t* self";
    for (my $i=0; $i<@$type; $i++) {
        print $file ", @$type[$i] arg_$i";
        if (@$size[$i] > 0) {
            print $file "[@$size[$i]]";
        }
    }
    print $file ");\n";
}

#
# 送信用関数宣言を出力する
#
sub print_send_func_defs
{
    my ($file, $menu, $node) = @_;
    my ($name, $type, $size) = split_node ($node);

    print "name: $name, type: @$type, size: @$size\n";

    print $file "int mesg_${port}_send_${menu}_${name} (mesg_${port}_t* self";

    for (my $i=0; $i<@$type; $i++) {
        print $file ", @$type[$i] arg_$i";
        if (@$size[$i] > 0) {
            print $file "[@$size[$i]]";
        }
    }
    print $file ");\n";
}

#
# 受信用関数宣言を出力する
#
sub print_recv_func_defs
{
    my ($file, $port, $arch, $menu) = @_;

    print $file "int mesg_${port}_recv_${menu} (mesg_${port}_t* self);\n";
}

#
# 送信用関数を出力する
#
sub print_send_func_decl
{
    my ($file, $port, $arch, $menu, $node) = @_;
    my ($name, $type, $size) = split_node ($node);

    print $file "\n";
    print $file "int mesg_${port}_send_${menu}_${name} (mesg_${port}_t* self";

    for (my $i=0; $i<@$type; $i++) {
        print $file ", @$type[$i] arg_$i";
        if (@$size[$i] > 0) {
            print $file "[@$size[$i]]";
        }
    }
    print $file ")\n";
    print $file "{\n";

    print_pack_save_args ($file, $menu, $name, $type, $size);

    print $file "    return mesg_${port}_send (self);\n";
    print $file "}\n";
}

#
# おおもとの受信用関数を出力する
#
sub print_recv_all_func_decl
{
    my ($file, $port, $arch, $menu) = @_;

    print $file "\n";
    print $file "int mesg_${port}_recv_${menu} (mesg_${port}_t* self)\n";
    print $file "{\n";
    print $file "    char menu[8];\n";
    print $file "    long mesg;\n";
    print $file "\n";
    print $file "    pack_load (self->recv_buff, \"c8 l\", menu, &mesg);\n";
    print $file "    mesg_${port}_recv_${menu}_func_vect[mesg] (self);\n";
    print $file "}\n";
}

#
# 受信用関数を出力する
#
sub print_recv_func_decl
{
    my ($file, $port, $arch, $menu, $node) = @_;
    my ($name, $type, $size) = split_node ($node);

    # 関数定義の出力
    print $file "\n";
    print $file "int mesg_${port}_recv_${menu}_${name} (mesg_${port}_t* self)\n";
    print $file "{\n";
    print $file "    char menu[8];\n";
    print $file "    long mesg;\n";
    print_vals_decl ($file, $menu, $name, $type, $size);
    print_pack_load_args ($file, $menu, $name, $type, $size);
    # handlerの呼び出し
    print_recv_proc ($file, $menu, $name, $type, $size);
    print $file "}\n";
}

# 受信用関数ベクタを出力
sub print_recv_func_vect_decl
{
    my ($file, $port, $arch, $menu, $node) = @_;
    my ($name, @args) = split_node ($node);

    print $file "    mesg_${port}_recv_${menu}_${name},\n";
}

#
# ローカル変数の定義を出力する
#
sub print_vals_decl
{
    my ($file, $base_name, $menu_name, $args_type_list, $args_size_list) = @_;

    for (my $i=0; $i<@$args_type_list; $i++) {
        if (@$args_type_list[$i] eq "void") {
            next;
        }
        print $file "    @$args_type_list[$i] arg_$i";
        if (@$args_size_list[$i] != 0) {
            print $file "[@$args_size_list[$i]]";
        }
        print $file ";\n";
    }
    print $file "\n";
}

#
# handlerの呼び出しの出力
#
sub print_recv_proc
{
    my ($file, $menu, $name, $args_type_list, $args_size_list) = @_;

    # print $file "#ifdef ",uc ($menu), "_", uc($name), "\n";

    print $file "    return ${menu}_${name} (self";
    for (my $i=0; $i<@$args_type_list; $i++) {
        print $file ", arg_$i";
    }
    print $file ");\n";
    #print $file "#else\n";
    #print $file "    return 0;\n";
    #print $file "#endif\n";
}

# 型名リストをlibpack用の書式文字列に変換して出力する
# 型名は double, short[12]など。
sub print_pack_format
{
    my ($file, $base_name, $menu_name, $args_type_list, $args_size_list) = @_;
    my %table = ("void"   => ' ',
                 "char"   => 'c',
                 "short"  => 'h',
                 "long"   => 'l',
                 "int"    => 'l',
                 "float"  => 'f',
                 "double" => 'd');
    # char[8]で名前をつける
    print $file "\"c8 l ";
    # 引数を書式文字列に変換
    for (my $i=0; $i<@$args_type_list; $i++) {
        if (@$args_type_list[$i] eq "void") {
            next;
        }
        print $file "$table{@$args_type_list[$i]}";
        if (@$args_size_list[$i] > 0) {
            print $file "@$args_size_list[$i]";
        }
        unless ($i==@$args_type_list-1) {
            print $file " ";
        }
    }
    print $file "\"";    
}

# 送信用関数のpack_save部分を出力する
sub print_pack_save_args
{
    my ($file, $menu, $name, $args_type_list, $args_size_list) = @_;

    print $file "    self->send_size = pack_size (";
    print_pack_format ($file, $menu, $name, $args_type_list, $args_size_list);
    print $file ");\n";


    print $file "    pack_save (self->send_buff, ";

    print_pack_format ($file, $menu, $name, $args_type_list, $args_size_list);

    # char[8]で名前を付加
    print $file ", \"", uc($menu), "\"";
    print $file ", MESG_", uc($menu), "_", uc($name);

    for (my $i=0; $i<@$args_type_list; $i++) {
        print $file ", arg_$i";
    }
    print $file ");\n";
}

# pack_load_argsを生成
sub print_pack_load_args
{
    my ($file, $menu, $name, $args_type_list, $args_size_list) = @_;

    print $file "    pack_load (self->recv_buff, ";

    print_pack_format ($file, $menu, $name, $args_type_list, $args_size_list);

    print $file ", menu";
    print $file ", &mesg";

    for (my $i=0; $i<@$args_type_list; $i++) {
        print $file ", ";
        if (@$args_size_list[$i] == 0) {
            print $file "&";
        }
        print $file "arg_$i";
    }
    print $file ");\n";
}

# node文字列を分解する
sub split_node
{
    # 引数用の局所変数
    my @type = (), @size = (), ($name, @args) = split (',', $_[0]);

    foreach my $i (@args) {
        # 書式の確認
        unless ($i =~ /(void|char|short|long|int|float|double)(?:\[(\d+)\])?/x) {
            die "Invalid argumnet format.\n";
        }
        if ($i eq "void") {
            @type = ();
            @size = ();
            return ($name, \@type, \@size);
        }
        push (@type, $1);
        if ($2 =='') {
            push (@size, 0);
        }else {
            push (@size, $2);
        }
    }
    return ($name, \@type, \@size);
}
