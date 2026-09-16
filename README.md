# Blue in Rhapsody

Alternative names considered included "Like a Tattoo" or "Rhapsody in C."


WASM compilation command
```
cd c
emcc -g -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wunused-parameter -Wunused-but-set-parameter -ferror-limit=100 -Iinclude src/meter.c -o bluerhapsody.mjs -sEXPORTED_FUNCTIONS=_int_sqrt -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sMODULARIZE -sEXPORT_NAME='createModule' -sENVIRONMENT='web' -sSINGLE_FILE=1
mv bluerhapsody.mjs ../react/wasm
```
