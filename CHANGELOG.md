# Changelog

## [0.0.3](https://github.com/jackrayner/d2dx/compare/0.0.2...0.0.3) (2026-10-24)


### Features

* Add `nokeepaspectratio` option ([b58d523](https://github.com/jackrayner/d2dx/commit/b58d523e38de4f3cbfba1266dba9e2088bca3521))
* use the full screen width for cutscenes and add nokeepaspectratio ([3038284](https://github.com/jackrayner/d2dx/commit/3038284c3e3714571327a68f05c350d0aa875ce8))


### Bug Fixes

* make the embedded SGD2FreeRes read D2DX's config file ([428df08](https://github.com/jackrayner/d2dx/commit/428df082acec09e1c5efce668cd5a3206bb1e560))
* update the embedded SGD2FreeRes to 3.0.3.1 (extended tile culling) ([4bb1089](https://github.com/jackrayner/d2dx/commit/4bb1089f5991f97f4f084a34f56cc944c27c8ce3))
* Use full screen size for cinematics ([6be5f67](https://github.com/jackrayner/d2dx/commit/6be5f67ca00b9ce9d62dd8fe6628ad7a08ce3ae3))

## [0.0.2](https://github.com/jackrayner/d2dx/compare/0.0.1...0.0.2) (2026-10-24)


### Bug Fixes

* fix precision lost when calculating the frame time ([99bf75f](https://github.com/jackrayner/d2dx/commit/99bf75fff1fe5d03ec35beb6a60f77f0c71265b3))
* initialise the previous frame time ([67771bf](https://github.com/jackrayner/d2dx/commit/67771bfe0c67adb09e85c63206eaaff13a55f96e))
* stop frame timing and unit IDs losing precision in long sessions ([75c3cbe](https://github.com/jackrayner/d2dx/commit/75c3cbedfa29ed680fe76451519bc964b3c7944b))
* use the full 32-bit unit ids and unit types in the motion predictor ([1471c16](https://github.com/jackrayner/d2dx/commit/1471c16ed47f449349475944d4a9bdfb9f2a0988))


### Performance Improvements

* shrink the GPU vertex buffer to 1M vertices ([5d62ad0](https://github.com/jackrayner/d2dx/commit/5d62ad0e758f1455023d1b0ecbdff4467068b7da))
* shrink the vertex buffer ([1361ff8](https://github.com/jackrayner/d2dx/commit/1361ff8af129a88198702c59bb5791c46c84d6ee))

## 0.0.1 (2026-10-24)


### Miscellaneous Chores

* release 0.0.1 ([e4eec1d](https://github.com/jackrayner/d2dx/commit/e4eec1ddb9606ebd346f0f4fe4092a8e002f5e50))
