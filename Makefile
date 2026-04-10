CC      = gcc
EMCC    = emcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
SRCS    = src/graph.c src/gen.c src/state.c src/traverse.c src/main.c

# Native build
native: $(SRCS)
	@mkdir -p build
	$(CC) $(CFLAGS) -o build/dream-graph $(SRCS)

# WebAssembly build
WASM_EXPORTS = _engine_init,_engine_current_node,_engine_node_name,\
_engine_node_role,_engine_refresh_links,_engine_link_display,\
_engine_link_passable,_engine_move,_engine_key_count,_engine_key_name,\
_engine_node_count,_engine_visited

wasm: $(SRCS)
	$(EMCC) $(CFLAGS) -o web/dream-graph.js $(SRCS) \
		-s EXPORTED_FUNCTIONS='[$(WASM_EXPORTS)]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString"]' \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s MODULARIZE=1 \
		-s EXPORT_NAME='DreamGraph'

clean:
	rm -rf build/dream-graph build/*.o web/dream-graph.js web/dream-graph.wasm

.PHONY: native wasm clean
