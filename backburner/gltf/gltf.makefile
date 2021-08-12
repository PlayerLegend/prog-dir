test/glb-toc: src/gltf/glb-toc.test.o src/gltf/gltf.o src/log/log.o src/file/file.o src/json/json.o
bin/glb-json: src/gltf/glb-json.util.o src/gltf/gltf.o src/log/log.o src/file/file.o src/json/json.o
bin/glb-info: src/gltf/glb-info.util.o src/gltf/gltf.o src/log/log.o src/file/file.o src/json/json.o

#TESTS_C += test/glb-toc
#UTILS_C += bin/glb-json bin/glb-info
