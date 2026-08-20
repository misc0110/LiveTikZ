# PGFPlots completion definitions
\begin{axis}
\begin{semilogxaxis}
\begin{semilogyaxis}
\begin{loglogaxis}
\addplot[options] coordinates {coordinates};
\addplot+[options] expression {expression};
\addplot3[options] coordinates {coordinates};
\addplot3+[options] table {file};
\addplot table[options] {file};
\addlegendentry{text}
\addlegendimage[options]{style}
\legend{entries}
\pgfplotstabletypeset[options]{file}
\pgfplotstableread[options]{file}{table}
\pgfplotstablegetelem{row}{column}\of{table}
\pgfplotsinvokeforeach{values}{commands}
\pgfplotsforeachungrouped{variable}{values}{commands}
\xlabel{text}
\ylabel{text}
\zlabel{text}
\title{text}
\caption{text}
\label{name}
\ref{name}
\coordinate (name) at (axis cs:x,y);
\draw[options] (coordinate) -- (coordinate);
\fillbetween[options]
\nextgroupplot[options]
\pgfplotsset{key=value}

# Frequently used PGFPlots keys
% axis lines, axis line style, axis equal, axis on top, width, height,
% xmin, xmax, ymin, ymax, xmode, ymode, xscale, yscale, xlabel, ylabel,
% xtick, ytick, xticklabels, yticklabels, xticklabel style, yticklabel style,
% grid, grid style, legend style, legend columns, legend pos, mark, mark size,
% color, thick, semithick, ultra thick, line width, error bars, nodes near coords,
% point meta, colormap, cycle list, table, x, y, z, restrict expr to domain,
% filter discard warning, unbounded coords, scaled ticks, compat
