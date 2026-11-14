# latexmk configuration: `latexmk` in this directory builds build/battleship-report.pdf
$pdf_mode = 1;                       # pdfLaTeX
$out_dir = 'build';
$jobname = 'battleship-report';
$pdflatex = 'pdflatex -interaction=nonstopmode -halt-on-error -file-line-error %O %S';
$bibtex_use = 2;                     # run bibtex when needed, clean its files
@default_files = ('main.tex');
