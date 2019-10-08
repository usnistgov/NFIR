
The Doxyfile is configured to write output to `.\doc\html` dir.  It references the css file:  `.\doc\nfir.css`.

See:

```
HTML_EXTRA_STYLESHEET = nfir.css
INPUT                 = ..\..\NFIR
OUTPUT_DIRECTORY      = ..\..\NFIR\doc
```

Run:
```
NFIR\doc> doxygen   <======
```