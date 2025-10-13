all: zgdb.js

zgdb.js: zgdb.cpp visutils.h zgdb.txt cpc.txt presteam.txt other.txt tagdef.txt roguelike.txt mygames.txt stores.txt philosophy.txt parser.cpp readdata.cpp
	em++ -O2 -std=c++17 zgdb.cpp -o zgdb.js \
          -s EXPORTED_FUNCTIONS="['_start', '_main_page', '_malloc', '_explain_tag', '_explain_group', '_explain_game', '_explain_by', '_find_games', '_on_find_change']" \
          -s EXPORTED_RUNTIME_METHODS='["FS","ccall"]' \
          --preload-file zgdb.txt \
          --preload-file cpc.txt \
          --preload-file presteam.txt \
          --preload-file other.txt \
          --preload-file tagdef.txt \
          --preload-file roguelike.txt \
          --preload-file mygames.txt \
          --preload-file stores.txt \
          --preload-file philosophy.txt \
          -fwasm-exceptions

test: zgdb.js
	cp index.html ~/public_html/zgdb
	cp zgdb.wasm ~/public_html/zgdb
	cp zgdb.data ~/public_html/zgdb
	cp zgdb.js ~/public_html/zgdb
	cp zeno.css ~/public_html/zgdb

# create a zip file to upload to itch.io

zgdb.zip: index.html zgdb.js zgdb.wasm zeno.css zgdb.data
	rm -rf zip
	mkdir -p zip
	cp -r pngs zip/
	cp index.html zip/
	cp zgdb.data zip/
	cp zgdb.js zip/
	cp zgdb.wasm zip/
	cp zeno.css zip/
	cd zip; zip -r ../zgdb.zip *

zip-itch/zgdb.js: zgdb.cpp visutils.h zgdb.txt zgdb.data
	mkdir -p zip-itch
	em++ -DITCH -O2 -std=c++11 zgdb.cpp -o zip-itch/zgdb.js \
          -s EXPORTED_FUNCTIONS="['_start', '_answer', '_next', '_results', '_back', '_back_to', '_help','_malloc', '_details', '_do_import', '_resetquiz', '_suggestions', '_on_keydown', '_compare', '_aggregate']" \
          -s EXTRA_EXPORTED_RUNTIME_METHODS='["FS","ccall"]' \
          --preload-file zgdb.txt --preload-file answers.txt

# need to manually edit index.html for now, and to copy PNGs if changed
zgdb-itch.zip: zip-itch/zgdb.js
	rm -rf zgdb-itch.zip
	cd zip-itch; zip -r ../zgdb-itch.zip *
