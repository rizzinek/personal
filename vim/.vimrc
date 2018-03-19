" vimrc by rzn

" don't bother with vi compatibility
set nocompatible

" add cp1251 to the list of file encodings
set fileencodings += "windows-1251"

""" indentation """
" copy indent from the current line
set autoindent
" c-style indent, for tweaking the beahviour look at cin* options
set cindent
" copy previous' line indentation
" set copyindent
" do not remove autoindentation after moving away from a newly created line
set cpoptions += "I"
" round indentation to a multiple of shiftwidth
set shiftround
" number of spaces for the indentation
set shiftwidth =4

""" tabs """
" do not expand tabs to spaces
set noexpandtab
" mainly to remove shiftwidth worth of spaces at ^ with Backspace
set smarttab
" tab in a file counts as 4 spaces
set tabstop =4

""" search """
" do not highlight all search matches
set nohlsearch
" incremental search matching
set incsearch
" ignore case if the whole search text is lowercase
set ignorecase
set smartcase



"automatically read file when it is modified externally
set autoread
" let vim choose colors that look good either on dark or on light screen
set background&
" insert mode completion options: complete*
" do not hide buffers when they are abandoned
set nohidden
" all windows should have a status bar
set laststatus =2
" do not redraw when excuting marcoses
set lazyredraw
" add characters to the list of stuff that gets in/decremented
set nrformats += "alpha"
" print line numbers
set number
" show cursor x and y position
set ruler
" when scrolling past the screen edges, show not a single line
" but half of a screen of text
set scrolljump =-50
" keep a minimum of 2 lines above and below the cursor at all times
set scrolloff =2
" when completing tags, show both the tag and the search pattern
set showfulltag
" allow virtual editing in visual mode
set virtualedit = "block"
" when completing commands, show a menu with all options
set wildmenu

set noswapfile

colorscheme desert
