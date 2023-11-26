[[back to main page]](../../README.md)

## Heap Allocation Logger

dependencies: `<iostream>`, `<cstdlib>`, `<unordered_map>`, `<execinfo.h>`, `<cxxabi.h>`;
___
#### usage:

#### 1. First(!) set a preprocessor flag `#define MEMLOG`
#### 2. Then `#include <memlog.h>`, alternatively include as part of the `<utilities.h>` library.
#### This will overide the `new` and `delete` keywords: anytime memory is allocated or deallocated on the heap,
#### this will be logged to the console (bytes, calling function, total bytes).
#### 3. The order above matters, because `<memlog.h>`will only compile (thus take effect) if the `MEMLOG` flag has
#### been defined beforehand. The benefit is that the logging can easily be disabled by simply commenting out the
#### `MEMLOG` define statement.