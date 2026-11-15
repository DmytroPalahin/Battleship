# latexmk configuration: `latexmk` in this directory builds the two editions of the report,
# build/battleship-report-fr.pdf (the original) and build/battleship-report-en.pdf.
$pdf_mode = 1;                       # pdfLaTeX
$out_dir = 'build';
$pdflatex = 'pdflatex -interaction=nonstopmode -halt-on-error -file-line-error %O %S';
$bibtex_use = 2;                     # run bibtex when needed, clean its files
@default_files = ('battleship-report-fr.tex', 'battleship-report-en.tex');
