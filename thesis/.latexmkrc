# The following is equivalent to using `latexmk -lualatex`
$pdf_mode = 4;
$dvi_mode = $postscript_mode = 0;

# The following adds support for the svg package
$lualatex_default_switches = '-shell-escape';

$do_cd = 1;
$out_dir = 'build';

# The following adds support for the glossaries package
# See: http://mirror.utexas.edu/ctan/support/latexmk/example_rcfiles/glossary_latexmkrc
add_cus_dep('glo', 'gls', 0, 'makeglossaries');
sub makeglossaries {
    my ($base_name, $path) = fileparse($_[0]);
    pushd $path;
    my $return = system "makeglossaries $base_name";
    popd;
    return $return;
}

@default_files = ('main.tex');
