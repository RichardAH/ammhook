all:
	wasmcc amm.c -o amm.wasm -Oz -Wno-int-conversion -Wno-incompatible-pointer-types -Wl,--allow-undefined -I/root/hookwork/
	wasm-opt amm.wasm -o amm.wasm -O3 
	hook-cleaner amm.wasm
	guard_checker amm.wasm
	
