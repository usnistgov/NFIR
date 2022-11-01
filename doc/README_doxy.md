## Configure

### Windows
See these Doxyfile (file) parameters:

```
PROJECT_NAME          = NFIR
HTML_EXTRA_STYLESHEET = nfir.css
INPUT                 = ..\..\NFIR  (on or near line 816)
OUTPUT_DIRECTORY      = ..\..\NFIR\doc
```

### Linux
Modify the Doxyfile parameters (above) to use forward-slash: `/`.

## Run

### Windows
The Doxyfile is configured to write output to `.\doc\html` dir.  It references the css file:  `.\doc\nfir.css`.

```
.\NFIR\doc> "C:\Program Files\doxygen\bin\doxygen.exe"
```

### Linux
The Doxyfile is configured to write output to `./doc/html` dir.  It references the css file:  `./doc/nfir.css`.

```
./NFIR/doc$ doxygen
```

## View in Browser
Navigate to file: `./doc/html/index.html` .
