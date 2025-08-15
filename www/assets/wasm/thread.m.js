// This code implements the `-sMODULARIZE` settings by taking the generated
// JS program code (INNER_JS_CODE) and wrapping it in a factory function.

// Single threaded MINIMAL_RUNTIME programs do not need access to
// document.currentScript, so a simple export declaration is enough.
var LosuThread = (() => {
  // When MODULARIZE this JS may be executed later,
  // after document.currentScript is gone, so we save it.
  // In EXPORT_ES6 mode we can just use 'import.meta.url'.
  var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;
  return async function(moduleArg = {}) {
    var moduleRtn;

// include: shell.js
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(moduleArg) => Promise<Module>
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
var Module = moduleArg;

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof WorkerGlobalScope != 'undefined';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = typeof process == 'object' && process.versions?.node && process.type != 'renderer';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)


var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

if (typeof __filename != 'undefined') { // Node
  _scriptName = __filename;
} else
if (ENVIRONMENT_IS_WORKER) {
  _scriptName = self.location.href;
}

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var readAsync, readBinary;

if (ENVIRONMENT_IS_NODE) {
  const isNode = typeof process == 'object' && process.versions?.node && process.type != 'renderer';
  if (!isNode) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  var nodeVersion = process.versions.node;
  var numericVersion = nodeVersion.split('.').slice(0, 3);
  numericVersion = (numericVersion[0] * 10000) + (numericVersion[1] * 100) + (numericVersion[2].split('-')[0] * 1);
  if (numericVersion < 160000) {
    throw new Error('This emscripten-generated code requires node v16.0.0 (detected v' + nodeVersion + ')');
  }

  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('fs');

  scriptDirectory = __dirname + '/';

// include: node_shell_read.js
readBinary = (filename) => {
  // We need to re-wrap `file://` strings to URLs.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename);
  assert(Buffer.isBuffer(ret));
  return ret;
};

readAsync = async (filename, binary = true) => {
  // See the comment in the `readBinary` function.
  filename = isFileURI(filename) ? new URL(filename) : filename;
  var ret = fs.readFileSync(filename, binary ? undefined : 'utf8');
  assert(binary ? Buffer.isBuffer(ret) : typeof ret == 'string');
  return ret;
};
// end include: node_shell_read.js
  if (process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  arguments_ = process.argv.slice(2);

  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };

} else
if (ENVIRONMENT_IS_SHELL) {

  const isNode = typeof process == 'object' && process.versions?.node && process.type != 'renderer';
  if (isNode || typeof window == 'object' || typeof WorkerGlobalScope != 'undefined') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

} else

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  try {
    scriptDirectory = new URL('.', _scriptName).href; // includes trailing slash
  } catch {
    // Must be a `blob:` or `data:` URL (e.g. `blob:http://site.com/etc/etc`), we cannot
    // infer anything from them.
  }

  if (!(typeof window == 'object' || typeof WorkerGlobalScope != 'undefined')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');

  {
// include: web_or_worker_shell_read.js
if (ENVIRONMENT_IS_WORKER) {
    readBinary = (url) => {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.responseType = 'arraybuffer';
      xhr.send(null);
      return new Uint8Array(/** @type{!ArrayBuffer} */(xhr.response));
    };
  }

  readAsync = async (url) => {
    // Fetch has some additional restrictions over XHR, like it can't be used on a file:// url.
    // See https://github.com/github/fetch/pull/92#issuecomment-140665932
    // Cordova or Electron apps are typically loaded from a file:// url.
    // So use XHR on webview if URL is a file URL.
    if (isFileURI(url)) {
      return new Promise((resolve, reject) => {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, true);
        xhr.responseType = 'arraybuffer';
        xhr.onload = () => {
          if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            resolve(xhr.response);
            return;
          }
          reject(xhr.status);
        };
        xhr.onerror = reject;
        xhr.send(null);
      });
    }
    var response = await fetch(url, { credentials: 'same-origin' });
    if (response.ok) {
      return response.arrayBuffer();
    }
    throw new Error(response.status + ' : ' + response.url);
  };
// end include: web_or_worker_shell_read.js
  }
} else
{
  throw new Error('environment detection error');
}

var out = console.log.bind(console);
var err = console.error.bind(console);

var IDBFS = 'IDBFS is no longer included by default; build with -lidbfs.js';
var PROXYFS = 'PROXYFS is no longer included by default; build with -lproxyfs.js';
var WORKERFS = 'WORKERFS is no longer included by default; build with -lworkerfs.js';
var FETCHFS = 'FETCHFS is no longer included by default; build with -lfetchfs.js';
var ICASEFS = 'ICASEFS is no longer included by default; build with -licasefs.js';
var JSFILEFS = 'JSFILEFS is no longer included by default; build with -ljsfilefs.js';
var OPFS = 'OPFS is no longer included by default; build with -lopfs.js';

var NODEFS = 'NODEFS is no longer included by default; build with -lnodefs.js';

// perform assertions in shell.js after we set up out() and err(), as otherwise
// if an assertion fails it cannot print the message

assert(!ENVIRONMENT_IS_SHELL, 'shell environment detected but not enabled at build time.  Add `shell` to `-sENVIRONMENT` to enable.');

// end include: shell.js

// include: preamble.js
// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

var wasmBinary;

if (typeof WebAssembly != 'object') {
  err('no native wasm support detected');
}

// Wasm globals

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

// In STRICT mode, we only define assert() when ASSERTIONS is set.  i.e. we
// don't define it at all in release modes.  This matches the behaviour of
// MINIMAL_RUNTIME.
// TODO(sbc): Make this the default even without STRICT enabled.
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed' + (text ? ': ' + text : ''));
  }
}

// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.

/**
 * Indicates whether filename is delivered via file protocol (as opposed to http/https)
 * @noinline
 */
var isFileURI = (filename) => filename.startsWith('file://');

// include: runtime_common.js
// include: runtime_stack_check.js
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  var max = _emscripten_stack_get_end();
  assert((max & 3) == 0);
  // If the stack ends at address zero we write our cookies 4 bytes into the
  // stack.  This prevents interference with SAFE_HEAP and ASAN which also
  // monitor writes to address zero.
  if (max == 0) {
    max += 4;
  }
  // The stack grow downwards towards _emscripten_stack_get_end.
  // We write cookies to the final two words in the stack and detect if they are
  // ever overwritten.
  HEAPU32[((max)>>2)] = 0x02135467;
  HEAPU32[(((max)+(4))>>2)] = 0x89BACDFE;
  // Also test the global address 0 for integrity.
  HEAPU32[((0)>>2)] = 1668509029;
}

function checkStackCookie() {
  if (ABORT) return;
  var max = _emscripten_stack_get_end();
  // See writeStackCookie().
  if (max == 0) {
    max += 4;
  }
  var cookie1 = HEAPU32[((max)>>2)];
  var cookie2 = HEAPU32[(((max)+(4))>>2)];
  if (cookie1 != 0x02135467 || cookie2 != 0x89BACDFE) {
    abort(`Stack overflow! Stack cookie has been overwritten at ${ptrToString(max)}, expected hex dwords 0x89BACDFE and 0x2135467, but received ${ptrToString(cookie2)} ${ptrToString(cookie1)}`);
  }
  // Also test the global address 0 for integrity.
  if (HEAPU32[((0)>>2)] != 0x63736d65 /* 'emsc' */) {
    abort('Runtime error: The application has corrupted its heap memory area (address zero)!');
  }
}
// end include: runtime_stack_check.js
// include: runtime_exceptions.js
// end include: runtime_exceptions.js
// include: runtime_debug.js
var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

// Used by XXXXX_DEBUG settings to output debug messages.
function dbg(...args) {
  if (!runtimeDebug && typeof runtimeDebug != 'undefined') return;
  // TODO(sbc): Make this configurable somehow.  Its not always convenient for
  // logging to show up as warnings.
  console.warn(...args);
}

// Endianness check
(() => {
  var h16 = new Int16Array(1);
  var h8 = new Int8Array(h16.buffer);
  h16[0] = 0x6373;
  if (h8[0] !== 0x73 || h8[1] !== 0x63) throw 'Runtime error: expected the system to be little-endian! (Run with -sSUPPORT_BIG_ENDIAN to bypass)';
})();

function consumedModuleProp(prop) {
  if (!Object.getOwnPropertyDescriptor(Module, prop)) {
    Object.defineProperty(Module, prop, {
      configurable: true,
      set() {
        abort(`Attempt to set \`Module.${prop}\` after it has already been processed.  This can happen, for example, when code is injected via '--post-js' rather than '--pre-js'`);

      }
    });
  }
}

function makeInvalidEarlyAccess(name) {
  return () => assert(false, `call to '${name}' via reference taken before Wasm module initialization`);

}

function ignoredModuleProp(prop) {
  if (Object.getOwnPropertyDescriptor(Module, prop)) {
    abort(`\`Module.${prop}\` was supplied but \`${prop}\` not included in INCOMING_MODULE_JS_API`);
  }
}

// forcing the filesystem exports a few things by default
function isExportedByForceFilesystem(name) {
  return name === 'FS_createPath' ||
         name === 'FS_createDataFile' ||
         name === 'FS_createPreloadedFile' ||
         name === 'FS_unlink' ||
         name === 'addRunDependency' ||
         // The old FS has some functionality that WasmFS lacks.
         name === 'FS_createLazyFile' ||
         name === 'FS_createDevice' ||
         name === 'removeRunDependency';
}

/**
 * Intercept access to a global symbol.  This enables us to give informative
 * warnings/errors when folks attempt to use symbols they did not include in
 * their build, or no symbols that no longer exist.
 */
function hookGlobalSymbolAccess(sym, func) {
  // In MODULARIZE mode the generated code runs inside a function scope and not
  // the global scope, and JavaScript does not provide access to function scopes
  // so we cannot dynamically modify the scrope using `defineProperty` in this
  // case.
  //
  // In this mode we simply ignore requests for `hookGlobalSymbolAccess`. Since
  // this is a debug-only feature, skipping it is not major issue.
}

function missingGlobal(sym, msg) {
  hookGlobalSymbolAccess(sym, () => {
    warnOnce(`\`${sym}\` is not longer defined by emscripten. ${msg}`);
  });
}

missingGlobal('buffer', 'Please use HEAP8.buffer or wasmMemory.buffer');
missingGlobal('asm', 'Please use wasmExports instead');

function missingLibrarySymbol(sym) {
  hookGlobalSymbolAccess(sym, () => {
    // Can't `abort()` here because it would break code that does runtime
    // checks.  e.g. `if (typeof SDL === 'undefined')`.
    var msg = `\`${sym}\` is a library symbol and not included by default; add it to your library.js __deps or to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE on the command line`;
    // DEFAULT_LIBRARY_FUNCS_TO_INCLUDE requires the name as it appears in
    // library.js, which means $name for a JS name with no prefix, or name
    // for a JS name like _name.
    var librarySymbol = sym;
    if (!librarySymbol.startsWith('_')) {
      librarySymbol = '$' + sym;
    }
    msg += ` (e.g. -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='${librarySymbol}')`;
    if (isExportedByForceFilesystem(sym)) {
      msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
    }
    warnOnce(msg);
  });

  // Any symbol that is not included from the JS library is also (by definition)
  // not exported on the Module object.
  unexportedRuntimeSymbol(sym);
}

function unexportedRuntimeSymbol(sym) {
  if (!Object.getOwnPropertyDescriptor(Module, sym)) {
    Object.defineProperty(Module, sym, {
      configurable: true,
      get() {
        var msg = `'${sym}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the Emscripten FAQ)`;
        if (isExportedByForceFilesystem(sym)) {
          msg += '. Alternatively, forcing filesystem support (-sFORCE_FILESYSTEM) can export this for you';
        }
        abort(msg);
      }
    });
  }
}

// end include: runtime_debug.js
var readyPromiseResolve, readyPromiseReject;

// Memory management

var wasmMemory;

var
/** @type {!Int8Array} */
  HEAP8,
/** @type {!Uint8Array} */
  HEAPU8,
/** @type {!Int16Array} */
  HEAP16,
/** @type {!Uint16Array} */
  HEAPU16,
/** @type {!Int32Array} */
  HEAP32,
/** @type {!Uint32Array} */
  HEAPU32,
/** @type {!Float32Array} */
  HEAPF32,
/** @type {!Float64Array} */
  HEAPF64;

// BigInt64Array type is not correctly defined in closure
var
/** not-@type {!BigInt64Array} */
  HEAP64,
/* BigUint64Array type is not correctly defined in closure
/** not-@type {!BigUint64Array} */
  HEAPU64;

var runtimeInitialized = false;



function updateMemoryViews() {
  var b = wasmMemory.buffer;
  HEAP8 = new Int8Array(b);
  HEAP16 = new Int16Array(b);
  HEAPU8 = new Uint8Array(b);
  HEAPU16 = new Uint16Array(b);
  HEAP32 = new Int32Array(b);
  HEAPU32 = new Uint32Array(b);
  HEAPF32 = new Float32Array(b);
  HEAPF64 = new Float64Array(b);
  HEAP64 = new BigInt64Array(b);
  HEAPU64 = new BigUint64Array(b);
}

// include: memoryprofiler.js
// end include: memoryprofiler.js
// end include: runtime_common.js
assert(typeof Int32Array != 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray != undefined && Int32Array.prototype.set != undefined,
       'JS engine does not provide full typed array support');

function preRun() {
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
  consumedModuleProp('preRun');
  // Begin ATPRERUNS hooks
  callRuntimeCallbacks(onPreRuns);
  // End ATPRERUNS hooks
}

function initRuntime() {
  assert(!runtimeInitialized);
  runtimeInitialized = true;

  checkStackCookie();

  // No ATINITS hooks

  wasmExports['__wasm_call_ctors']();

  // No ATPOSTCTORS hooks
}

function postRun() {
  checkStackCookie();
   // PThreads reuse the runtime from the main thread.

  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
  consumedModuleProp('postRun');

  // Begin ATPOSTRUNS hooks
  callRuntimeCallbacks(onPostRuns);
  // End ATPOSTRUNS hooks
}

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// Module.preRun (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
var runDependencyTracking = {};
var runDependencyWatcher = null;

function addRunDependency(id) {
  runDependencies++;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval != 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(() => {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            err('still waiting on run dependencies:');
          }
          err(`dependency: ${dep}`);
        }
        if (shown) {
          err('(end of list)');
        }
      }, 10000);
    }
  } else {
    err('warning: run dependency added without ID');
  }
}

function removeRunDependency(id) {
  runDependencies--;

  Module['monitorRunDependencies']?.(runDependencies);

  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    err('warning: run dependency removed without ID');
  }
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

/** @param {string|number=} what */
function abort(what) {
  Module['onAbort']?.(what);

  what = 'Aborted(' + what + ')';
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);

  ABORT = true;

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  // FIXME This approach does not work in Wasm EH because it currently does not assume
  // all RuntimeErrors are from traps; it decides whether a RuntimeError is from
  // a trap or not based on a hidden field within the object. So at the moment
  // we don't have a way of throwing a wasm trap from JS. TODO Make a JS API that
  // allows this in the wasm spec.

  // Suppress closure compiler warning here. Closure compiler's builtin extern
  // definition for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
  /** @suppress {checkTypes} */
  var e = new WebAssembly.RuntimeError(what);

  readyPromiseReject?.(e);
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

// show errors on likely calls to FS when it was not included
var FS = {
  error() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with -sFORCE_FILESYSTEM');
  },
  init() { FS.error() },
  createDataFile() { FS.error() },
  createPreloadedFile() { FS.error() },
  createLazyFile() { FS.error() },
  open() { FS.error() },
  mkdev() { FS.error() },
  registerDevice() { FS.error() },
  analyzePath() { FS.error() },

  ErrnoError() { FS.error() },
};


function createExportWrapper(name, nargs) {
  return (...args) => {
    assert(runtimeInitialized, `native function \`${name}\` called before runtime initialization`);
    var f = wasmExports[name];
    assert(f, `exported native function \`${name}\` not found`);
    // Only assert for too many arguments. Too few can be valid since the missing arguments will be zero filled.
    assert(args.length <= nargs, `native function \`${name}\` called with ${args.length} args but expects ${nargs}`);
    return f(...args);
  };
}

var wasmBinaryFile;

function findWasmBinary() {
  return base64Decode('AGFzbQEAAAABkgEWYAN/f38Bf2ADf35/AX5gBn98f39/fwF/YAJ/fwBgBH9/f38Bf2AAAGABfwF/YAR/fn9/AX9gAn9/AX9gA39/fwBgAX8AYAABf2AFf39/f38AYAJ8fwF8YAV/f39/fwF/YAd/f39/f39/AX9gBH9/f38AYAN+f38Bf2ACfn8Bf2ABfAF+YAR/fn5/AGACfn4BfAKTAQUWd2FzaV9zbmFwc2hvdF9wcmV2aWV3MQhmZF93cml0ZQAEA2VudglfYWJvcnRfanMABRZ3YXNpX3NuYXBzaG90X3ByZXZpZXcxCGZkX2Nsb3NlAAYWd2FzaV9zbmFwc2hvdF9wcmV2aWV3MQdmZF9zZWVrAAcDZW52FmVtc2NyaXB0ZW5fcmVzaXplX2hlYXAABgNoZwUIAwkKBQMGAwQLCwsLCwUFAwoFDAoFBQUFCgUGCgYKCgsFCAAGAQYAAAAICAAACAgICAgGCAsNAAAADg8JBhAREhIMAAIDEwQABgsLCwUACAUGBgEBBgAKCwYFCwsLFBQVCgYLCAYEBQFwAQkJBQcBAYICgIACBhIDfwFBgIAEC38BQQALfwFBAAsH+wIVBm1lbW9yeQIAEV9fd2FzbV9jYWxsX2N0b3JzAAUGZmZsdXNoACMGbWFsbG9jAFsLdGhyZWFkX2RlbW8AGglkZW1vX2ZjZnMAGwhkZW1vX3NqZgAcDWRlbW9fcHJpb3JpdHkAHRBkZW1vX3JvdW5kX3JvYmluAB4DcnVuAB8MZGVmYXVsdF9kZW1vACAZX19pbmRpcmVjdF9mdW5jdGlvbl90YWJsZQEACHN0cmVycm9yAGsYZW1zY3JpcHRlbl9zdGFja19nZXRfZW5kAGMZZW1zY3JpcHRlbl9zdGFja19nZXRfYmFzZQBiBGZyZWUAXRVlbXNjcmlwdGVuX3N0YWNrX2luaXQAYBllbXNjcmlwdGVuX3N0YWNrX2dldF9mcmVlAGEZX2Vtc2NyaXB0ZW5fc3RhY2tfcmVzdG9yZQBnF19lbXNjcmlwdGVuX3N0YWNrX2FsbG9jAGgcZW1zY3JpcHRlbl9zdGFja19nZXRfY3VycmVudABpCQ4BAEEBCwgqKStKS05YWgqu7gFnBgAQYBBTCyABAX8jgICAgABBEGshAiACIAA2AgwgAiABNgIIQQAPC4MBAQR/I4CAgIAAQaAIayECIAIkgICAgAAgAiAANgKcCCACIAE2AgwgAkEQaiEDIAIoApwIIQQgAigCDCEFIANBgAggBCAFEM2AgIAAGiACIAJBEGo2AgBB7oKEgAAgAhCogICAABpBACgCxJCEgAAQo4CAgAAaIAJBoAhqJICAgIAADwvVAQEIfyOAgICAAEEwayEDIAMkgICAgAAgAyAANgIsIAMgATYCKCADIAI2AiRBACEEIAQoAqCQhIAAIQUgA0EgaiAFNgIAIAMgBCkDmJCEgAA3AxggAyAEKQOQkISAADcDECADKAIsQQRqIQYgAygCLCgCACEHIAMoAighCCADQRBqIAhBAnRqKAIAIQkgAygCJCEKIAMgA0EQaiAKQQJ0aigCADYCDCADIAk2AgggAyAHNgIEIAMgBjYCAEHvh4SAACADEIeAgIAAIANBMGokgICAgAAPC3gBBH8jgICAgABBIGshASABJICAgIAAIAEgADYCHCABKAIcQQRqIQIgASgCHCgCACEDIAEoAhwoAkghBCABIAEoAhwoAlA2AgwgASAENgIIIAEgAzYCBCABIAI2AgBB946EgAAgARCHgICAACABQSBqJICAgIAADwv+AQEDfyOAgICAAEHgAGshACAAJICAgIAAQdyMhIAAIQFBACECIAEgAhCHgICAACAAIAIoAriohIAANgJAQZ6KhIAAIABBwABqEIeAgIAAIAAgAigCwKiEgAA2AjBB1oqEgAAgAEEwahCHgICAACAAIAIqAsiohIAAuzkDIEGriISAACAAQSBqEIeAgIAAIAAgAioCzKiEgAC7OQMQQd+IhIAAIABBEGoQh4CAgAAgACACKgLQqISAALs5AwBBxYiEgAAgABCHgICAACAAIAIqAtSohIAAQwAAyEKUuzkDUEHvj4SAACAAQdAAahCHgICAACAAQeAAaiSAgICAAA8LoAEBA38jgICAgABBEGshAiACIAA2AgwgAiABNgIIAkACQCACKAIMKAIAQQBGQQFxRQ0AIAIoAgghAyACKAIMIAM2AgAgAigCCEEANgJwDAELIAIgAigCDCgCADYCBAJAA0AgAigCBCgCcEEAR0EBcUUNASACIAIoAgQoAnA2AgQMAAsLIAIoAgghBCACKAIEIAQ2AnAgAigCCEEANgJwCw8LcQECfyOAgICAAEEQayEBIAEgADYC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QQA2AhggAEEANgIUIABBADYCECAAQQAoAqyohIAANgIMAkADQCAAKAIMQQBHQQFxRQ0BIAAgACgCHEEBajYCHCAAIAAoAgwoAmQgACgCGGo2AhggACAAKAIMKAJgIAAoAhRqNgIUIAAgACgCDCgCaCAAKAIQajYCECAAIAAoAgwoAnA2AgwMAAsLAkAgACgCHEEASkEBcUUNACAAKAIYsiAAKAIcspUhAUEAIAE4AsiohIAAIAAoAhSyIAAoAhyylSECQQAgAjgCzKiEgAAgACgCELIgACgCHLKVIQNBACADOALQqISAACAAQQA2AgggAEEAKAKsqISAADYCDAJAA0AgACgCDEEAR0EBcUUNASAAIAAoAgwoAkwgACgCCGo2AgggACAAKAIMKAJwNgIMDAALCyAAKAIIskEAKAK4qISAALKVIQRBACAEOALUqISAAAsPC4MDAwF/AX4KfyOAgICAAEHAAGshAiACJICAgIAAIAIgADYCPCACIAE2AjhCACEDQQAhBCAEIAM3AtCohIAAIAQgAzcCyKiEgAAgBCADNwLAqISAACAEIAM3AriohIAAIAQgAzcCsKiEgAAgBCADNwKoqISAACAEIAM3AqCohIAAIAIoAjwhBUEAIAU2ArCohIAAIAIoAjghBkEAIAY2ArSohIAAQQAhB0EAIAc2AriohIAAQQAhCEEAIAg2AryohIAAQQAhCUEAIAk2AsCohIAAQQAhCkEAIAo2AsSohIAAQQAhCyALKALAkISAACEMIAJBMGogDDYCACACIAspA7iQhIAANwMoIAIgCykDsJCEgAA3AyAgAigCPCENIAIgAkEgaiANQQJ0aigCADYCEEH9goSAACACQRBqEIeAgIAAAkAgAigCPEECRkEBcUUNACACIAIoAjg2AgBBzIOEgAAgAhCHgICAAAtBg5CEgABBABCHgICAACACQcAAaiSAgICAAA8LfgECfyOAgICAAEEQayEBIAEkgICAgAAgASAANgIMAkACQCABKAIMQQBGQQFxRQ0ADAELIAEgASgCDCgCRDYCCCABKAIMQQE2AkQgASgCDCABKAIIQQEQiICAgAAgASgCDCECQaCohIAAIAIQi4CAgAALIAFBEGokgICAgAAPC8cBAQV/I4CAgIAAQRBrIQAgACSAgICAAEGtjoSAAEEAEIeAgIAAA0BBACgC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gIAAGkG+hISAAEEBQQNBABCOgICAACEwIAUoAhAhMSAFKAIMITIgMigCACEzIDIgM0EBajYCACAxIDNBAnRqIDA2AgBBsISEgABBAkEEQQAQjoCAgAAhNCAFKAIQITUgBSgCDCE2IDYoAgAhNyA2IDdBAWo2AgAgNSA3QQJ0aiA0NgIAQaKEhIAAQQFBAkEAEI6AgIAAITggBSgCECE5IAUoAgwhOiA6KAIAITsgOiA7QQFqNgIAIDkgO0ECdGogODYCAAsgBUEgaiSAgICAAA8LgQMBBX8jgICAgABB0ABrIQEgASSAgICAACABIAA2AkxBqouEgABBABCogICAABpByo6EgABBABCogICAABoCQAJAAkAgASgCTEEAR0EBcUUNACABKAJMEKyAgIAADQELQdWEhIAAQQAQqICAgAAaQYmFhIAAQQAQqICAgAAaDAELIAEgASgCTDYCAEGZiISAACABEKiAgIAAGkHxioSAAEEAEKiAgIAAGiABKAJMIQIgAUEQaiEDIAIgAUHIAGogAUHEAGogAyABQQxqEJmAgIAAAkAgASgCDA0AQcaFhIAAQQAQqICAgAAaDAELIAEoAkggASgCRBCWgICAACABQQA2AggCQANAIAEoAgggASgCDEhBAXFFDQEgASgCCCEEIAFBEGogBEECdGooAgBBADYCVCABKAIIIQUgAUEQaiAFQQJ0aigCABCXgICAACABIAEoAghBAWo2AggMAAsLEJiAgIAAQfiMhIAAQQAQqICAgAAaCyABQdAAaiSAgICAAA8L6AEBCX8jgICAgABBEGshACAAJICAgIAAQYCMhIAAQQAQqICAgAAaQQAhASABIAEQloCAgABBmoSEgAAhAkEAIQMgACACIANBBCADEI6AgIAANgIEQZKEhIAAIQRBACEFIAAgBCAFQQMgBRCOgICAADYCCEGKhISAACEGQQAhByAAIAYgB0ECIAcQjoCAgAA2AgwgAEEANgIAAkADQCAAKAIAQQNIQQFxRQ0BIAAoAgAhCCAAQQRqIAhBAnRqKAIAEJeAgIAAIAAgACgCAEEBajYCAAwACwsQmICAgAAgAEEQaiSAgICAAA8L5AEBCH8jgICAgABBEGshACAAJICAgIAAQa2MhIAAQQAQqICAgAAaQQFBABCWgICAAEHXgISAACEBQQAhAiAAIAEgAkECIAIQjoCAgAA2AgRBw4CEgAAhA0EAIQQgACADIARBCCAEEI6AgIAANgIIQc2AhIAAIQVBACEGIAAgBSAGQQQgBhCOgICAADYCDCAAQQA2AgACQANAIAAoAgBBA0hBAXFFDQEgACgCACEHIABBBGogB0ECdGooAgAQl4CAgAAgACAAKAIAQQFqNgIADAALCxCYgICAACAAQRBqJICAgIAADwvMAQECfyOAgICAAEEQayEAIAAkgICAgABBi4uEgABBABCogICAABpBA0EAEJaAgIAAIABBr4CEgABBAUEGQQAQjoCAgAA2AgQgAEGigISAAEEFQQJBABCOgICAADYCCCAAQZWAhIAAQQNBBEEAEI6AgIAANgIMIABBADYCAAJAA0AgACgCAEEDSEEBcUUNASAAKAIAIQEgAEEEaiABQQJ0aigCABCXgICAACAAIAAoAgBBAWo2AgAMAAsLEJiAgIAAIABBEGokgICAgAAPC+gBAQl/I4CAgIAAQRBrIQAgACSAgICAAEHMi4SAAEEAEKiAgIAAGkECIQEgASABEJaAgIAAQcSEhIAAIQJBACEDIAAgAiADQQUgAxCOgICAADYCBEG2hISAACEEQQAhBSAAIAQgBUEDIAUQjoCAgAA2AghBqISEgAAhBkEAIQcgACAGIAdBBCAHEI6AgIAANgIMIABBADYCAAJAA0AgACgCAEEDSEEBcUUNASAAKAIAIQggAEEEaiAIQQJ0aigCABCXgICAACAAIAAoAgBBAWo2AgAMAAsLEJiAgIAAIABBEGokgICAgAAPCzUBAX8jgICAgABBEGshASABJICAgIAAIAEgADYCDCABKAIMEJqAgIAAIAFBEGokgICAgAAPCw8AQcuBhIAAEJqAgIAADwsEAEEBCwIAC/sCAQN/AkAgAA0AQQAhAQJAQQAoAvimhIAARQ0AQQAoAvimhIAAEKOAgIAAIQELAkBBACgCkKiEgABFDQBBACgCkKiEgAAQo4CAgAAgAXIhAQsCQBCmgICAACgCACIARQ0AA0ACQAJAIAAoAkxBAE4NAEEBIQIMAQsgABChgICAAEUhAgsCQCAAKAIUIAAoAhxGDQAgABCjgICAACABciEBCwJAIAINACAAEKKAgIAACyAAKAI4IgANAAsLEKeAgIAAIAEPCwJAAkAgACgCTEEATg0AQQEhAgwBCyAAEKGAgIAARSECCwJAAkACQCAAKAIUIAAoAhxGDQAgAEEAQQAgACgCJBGAgICAAICAgIAAGiAAKAIUDQBBfyEBIAJFDQEMAgsCQCAAKAIEIgEgACgCCCIDRg0AIAAgASADa6xBASAAKAIoEYGAgIAAgICAgAAaC0EAIQEgAEEANgIcIABCADcDECAAQgA3AgQgAg0BCyAAEKKAgIAACyABCwIACwIACxQAQdiohIAAEKSAgIAAQdyohIAACw4AQdiohIAAEKWAgIAACzsBAX8jgICAgABBEGsiAiSAgICAACACIAE2AgxB6KWEgAAgACABEMmAgIAAIQEgAkEQaiSAgICAACABC4EDAQd/I4CAgIAAQSBrIgMkgICAgAAgAyAAKAIcIgQ2AhAgACgCFCEFIAMgAjYCHCADIAE2AhggAyAFIARrIgE2AhQgASACaiEGIANBEGohBEECIQcCQAJAAkACQAJAIAAoAjwgA0EQakECIANBDGoQgICAgAAQz4CAgABFDQAgBCEFDAELA0AgBiADKAIMIgFGDQICQCABQX9KDQAgBCEFDAQLIARBCEEAIAEgBCgCBCIISyIJG2oiBSAFKAIAIAEgCEEAIAkbayIIajYCACAEQQxBBCAJG2oiBCAEKAIAIAhrNgIAIAYgAWshBiAFIQQgACgCPCAFIAcgCWsiByADQQxqEICAgIAAEM+AgIAARQ0ACwsgBkF/Rw0BCyAAIAAoAiwiATYCHCAAIAE2AhQgACABIAAoAjBqNgIQIAIhAQwBC0EAIQEgAEEANgIcIABCADcDECAAIAAoAgBBIHI2AgAgB0ECRg0AIAIgBSgCBGshAQsgA0EgaiSAgICAACABCwQAQQALBABCAAuHAQEDfyAAIQECQAJAIABBA3FFDQACQCAALQAADQAgACAAaw8LIAAhAQNAIAFBAWoiAUEDcUUNASABLQAADQAMAgsLA0AgASICQQRqIQFBgIKECCACKAIAIgNrIANyQYCBgoR4cUGAgYKEeEYNAAsDQCACIgFBAWohAiABLQAADQALCyABIABrC/ICAgN/AX4CQCACRQ0AIAAgAToAACAAIAJqIgNBf2ogAToAACACQQNJDQAgACABOgACIAAgAToAASADQX1qIAE6AAAgA0F+aiABOgAAIAJBB0kNACAAIAE6AAMgA0F8aiABOgAAIAJB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C0EwOgAAIAYoAiwhDAsgCUECciEZIAVBIHEhFCALQX5qIhogBUEPajoAACALQX9qQS1BKyAMQQBIGzoAACADQQFIIARBCHFFcSETIAZBEGohDANAIAwiCyAB/AIiDEGglISAAGotAAAgFHI6AAAgASAMt6FEAAAAAAAAMECiIQECQCALQQFqIgwgBkEQamtBAUcNACABRAAAAAAAAAAAYSATcQ0AIAtBLjoAASALQQJqIQwLIAFEAAAAAAAAAABiDQALQX8hDSADQf3///8HIBkgDiAaayIUaiITa0oNACAAQSAgAiATIANBAmogDCAGQRBqayILIAtBfmogA0gbIAsgAxsiA2oiDCAEEMiAgIAAIAAgFyAZEMKAgIAAIABBMCACIAwgBEGAgARzEMiAgIAAIAAgBkEQaiALEMKAgIAAIABBMCADIAtrQQBBABDIgICAACAAIBogFBDCgICAACAAQSAgAiAMIARBgMAAcxDIgICAACACIAwgAiAMShshDQsgBkGwBGokgICAgAAgDQsuAQF/IAEgASgCAEEHakF4cSICQRBqNgIAIAAgAikDACACKQMIEOaAgIAAOQMACwUAIAC9C6MBAQJ/I4CAgIAAQaABayIEJICAgIAAIAQgACAEQZ4BaiABGyIANgKUASAEQQAgAUF/aiIFIAUgAUsbNgKYAQJAQZABRQ0AIARBAEGQAfwLAAsgBEF/NgJMIARBhoCAgAA2AiQgBEF/NgJQIAQgBEGfAWo2AiwgBCAEQZQBajYCVCAAQQA6AAAgBCACIAMQyYCAgAAhASAEQaABaiSAgICAACABC7YBAQV/IAAoAlQiAygCACEEAkAgAygCBCIFIAAoAhQgACgCHCIGayIHIAUgB0kbIgdFDQAgBCAGIAcQvoCAgAAaIAMgAygCACAHaiIENgIAIAMgAygCBCAHayIFNgIECwJAIAUgAiAFIAJJGyIFRQ0AIAQgASAFEL6AgIAAGiADIAMoAgAgBWoiBDYCACADIAMoAgQgBWs2AgQLIARBADoAACAAIAAoAiwiAzYCHCAAIAM2AhQgAgsZAAJAIAANAEEADwsQu4CAgAAgADYCAEF/CwQAQSoLCAAQ0ICAgAALCABBpLGEgAALXQEBf0EAQYyxhIAANgKEsoSAABDRgICAACEAQQBBgICEgABBgICAgABrNgLcsYSA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ACABQcAAIANrrYggAiADrSIEhoQhAiABIASGIQELIAAgATcDACAAIAI3AwgLUwEBfgJAAkAgA0HAAHFFDQAgAiADQUBqrYghAUIAIQIMAQsgA0UNACACQcAAIANrrYYgASADrSIEiIQhASACIASIIQILIAAgATcDACAAIAI3AwgLmwQDAX8CfgR/I4CAgIAAQSBrIgIkgICAgAAgAUL///////8/gyEDAkACQCABQjCIQv//AYMiBKciBUH/h39qQf0PSw0AIABCPIggA0IEhoQhAyAFQYCIf2qtIQQCQAJAIABC//////////8PgyIAQoGAgICAgICACFQNACADQgF8IQMMAQsgAEKAgICAgICAgAhSDQAgA0IBgyADfCEDC0IAIAMgA0L/////////B1YiBRshACAFrSAEfCEDDAELAkAgACADhFANACAEQv//AVINACAAQjyIIANCBIaEQoCAgICAgIAEhCEAQv8PIQMMAQsCQCAFQf6HAU0NAEL/DyEDQgAhAAwBCwJAQYD4AEGB+AAgBFAiBhsiByAFayIIQfAATA0AQgAhAEIAIQMMAQsgAkEQaiAAIAMgA0KAgICAgIDAAIQgBhsiA0GAASAIaxDkgICAACACIAAgAyAIEOWAgIAAIAIpAwAiA0I8iCACKQMIQgSGhCEAAkACQCADQv//////////D4MgByAFRyACKQMQIAIpAxiEQgBSca2EIgNCgYCAgICAgIAIVA0AIABCAXwhAAwBCyADQoCAgICAgICACFINACAAQgGDIAB8IQALIABCgICAgICAgAiFIAAgAEL/////////B1YiBRshACAFrSEDCyACQSBqJICAgIAAIANCNIYgAUKAgICAgICAgIB/g4QgAIS/CwoAIAAkgICAgAALGgECfyOAgICAACAAa0FwcSIBJICAgIAAIAELCAAjgICAgAALHgBBACAAIABBmQFLG0EBdC8BsKOEgABBsJSEgABqCwwAIAAgABDqgICAAAsLpSgCAEGAgAQL5CXmlrDlu7oA6L2u6L2sAOWwsee7qgDkuK3kvJjlhYjnuqcA6auY5LyY5YWI57qnAOS9juS8mOWFiOe6pwDnu4jmraIA6ZW/5Lu75YqhAOS4reS7u+WKoQDnn63ku7vliqEA5Lit5LyY5YWI57qn5Lu75YqhAOmrmOS8mOWFiOe6p+S7u+WKoQDkvY7kvJjlhYjnuqfku7vliqEA5Lit562J5Lu75YqhAOWFiOadpeWFiOacjeWKoQDpmLvloZ4A5pyA55+t5L2c5LiaAC8vIOm7mOiupOe6v+eoi+iwg+W6pua8lOekugovLyDmvJTnpLrml7bpl7TniYfova7ovazosIPluqbnrpfms5UA6L+Q6KGMAOaXtumXtOeJhwBsb3dfcHJpb3JpdHkAbWVkaXVtX3ByaW9yaXR5AGhpZ2hfcHJpb3JpdHkAUHJpb3JpdHkALSsgICAwWDB4AC0wWCswWCAwWC0weCsweCAweABb6LCD5bqm5ZmoXSAlcwDliJ3lp4vljJbosIPluqblmajvvIznrpfms5U6ICVzAG5hbgBzaG9ydF90YXNrAG1lZGl1bV90YXNrAGxvbmdfdGFzawBpbmYAUm91bmQALCDml7bpl7TniYc6ICVkAHRhc2tfYwB0YXNrX2IAdGFza19hAEZDRlMAUlIATUxGUQBOQU4ASU5GAFNKRgDku7vliqFDAOS7u+WKoUIA5Lu75YqhQQDpu5jorqTku7vliqEzAOm7mOiupOS7u+WKoTIA6buY6K6k5Lu75YqhMQAuAChudWxsKQDplJnor686IOivt+i+k+WFpee6v+eoi+iwg+W6puS7o+eggeWGjei/kOihjOa8lOekugoA5o+Q56S6OiDku6PnoIHkuK3lupTljIXlkKvku7vliqHlrprkuYnlkozosIPluqbnrpfms5Xkv6Hmga8KAOmUmeivrzog5pyq6IO95LuO5Luj56CB5Lit6Kej5p6Q5Ye65pyJ5pWI55qE5Lu75YqhCgDmqKHmi5/ml7bpl7TotoXml7bvvIzlvLrliLbnu5PmnZ8KAOajgOa1i+WIsOaXtumXtOeJh+i9rui9rOiwg+W6pueul+azlQoA5qOA5rWL5Yiw5LyY5YWI57qn6LCD5bqm566X5rOVCgDmo4DmtYvliLBGQ0ZT6LCD5bqm566X5rOVCgDmo4DmtYvliLBTSkbosIPluqbnrpfms5UKAOacquajgOa1i+WIsOeJueWumuS7u+WKoe+8jOS9v+eUqOm7mOiupOS7u+WKoembhgoA6ZSZ6K+vOiDor7fovpPlhaXnur/nqIvosIPluqbku6PnoIEKAOe6v+eoiyAlcyAoSUQ6JWQpIOeKtuaAgeWPmOWMljogJXMgLT4gJXMKAOi+k+WFpeS7o+eggToKJXMKAOW5s+Wdh+WRqOi9rOaXtumXtDogJS4yZgoA5bmz5Z2H5ZON5bqU5pe26Ze0OiAlLjJmCgDlubPlnYfnrYnlvoXml7bpl7Q6ICUuMmYKAOaJp+ihjOe6v+eoiyAlcywg5Ymp5L2Z5pe26Ze0OiAlZAoAICDmiafooYzml7bpl7Q6ICVkLCDlkajovazml7bpl7Q6ICVkLCDnrYnlvoXml7bpl7Q6ICVkLCDlk43lupTml7bpl7Q6ICVkCgAgIOWIsOi+vuaXtumXtDogJWQsIOW8gOWni+aXtumXtDogJWQsIOWujOaIkOaXtumXtDogJWQKAOW9k+WJjeaXtumXtDogJWQKACAg5Yiw6L6+5pe26Ze0OiAlZCwg5byA5aeL5pe26Ze0OiAlZAoA5LiK5LiL5paH5YiH5o2i5qyh5pWwOiAlZAoAPT09PT09PT09PT09PT09PT09PT09PT09CgA9PT0g5LyY5YWI57qn6LCD5bqm5ryU56S6ID09PQoAPT09IOa0m+S5pue6v+eoi+iwg+W6pua8lOekuiA9PT0KAD09PSBSb3VuZCBSb2JpbiAo5pe26Ze054mH6L2u6L2sKSDosIPluqbmvJTnpLogPT09CgA9PT0gRkNGUyAo5YWI5p2l5YWI5pyN5YqhKSDosIPluqbmvJTnpLogPT09CgA9PT0gU0pGICjmnIDnn63kvZzkuJrkvJjlhYgpIOiwg+W6pua8lOekuiA9PT0KAD09PSDosIPluqbnu5/orqHkv6Hmga8gPT09CgAKPT09IOe6v+eoi+iwg+W6pua8lOekuuWujOaIkCA9PT0KAOe6v+eoiyAlcyDlrozmiJDmiafooYzor6bmg4U6CgDorablkYrvvJrnur/nqIsgJXMg562J5b6F5pe26Ze05Li66LSf5pWw77yM6YeN572u5Li6MAoA6K2m5ZGK77ya57q/56iLICVzIOWTjeW6lOaXtumXtOS4uui0n+aVsCAoJWQp77yM6YeN572u5Li6MAoA5byA5aeL57q/56iL6LCD5bqm5qih5oufLi4uCgDliIbmnpDovpPlhaXku6PnoIEuLi4KAC0tLSDml7bpl7TniYcgJWQgLS0tCgDpgInmi6nmiafooYznur/nqIs6ICVzIChJRDolZCwg5LyY5YWI57qnOiVkLCDliankvZnml7bpl7Q6JWQpCgDliJvlu7rnur/nqIs6ICVzIChJRDolZCwg5LyY5YWI57qnOiVkLCDmiafooYzml7bpl7Q6JWQpCgBDUFXliKnnlKjnjoc6ICUuMmYlJQoAAAAAAAAAAAAAAAAAAAEADgABAAwBAQC3AAEAPAABAAAAAAAAAAAAAAAAAPEBAQAGAgEA9gEBAEgBAQD5AQEA6BIBAAAAAAAAAAAAGQALABkZGQAAAAAFAAAAAAAACQAAAAALAAAAAAAAAAAZAAoKGRkZAwoHAAEACQsYAAAJBgsAAAsABhkAAAAZGRkAAAAAAAAAAAAAAAAAAAAADgAAAAAAAAAAGQALDRkZGQANAAACAAkOAAAACQAOAAAOAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAwAAAAAAAAAAAAAABMAAAAAEwAAAAAJDAAAAAAADAAADAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAPAAAABA8AAAAACRAAAAAAABAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEgAAAAAAAAAAAAAAEQAAAAARAAAAAAkSAAAAAAASAAASAAAaAAAAGhoaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABoAAAAaGhoAAAAAAAAJAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUAAAAAAAAAAAAAAAXAAAAABcAAAAACRQAAAAAABQAABQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFgAAAAAAAAAAAAAAFQAAAAAVAAAAAAkWAAAAAAAWAAAWAAAwMTIzNDU2Nzg5QUJDREVGTm8gZXJyb3IgaW5mb3JtYXRpb24ASWxsZWdhbCBieXRlIHNlcXVlbmNlAERvbWFpbiBlcnJvcgBSZXN1bHQgbm90IHJlcHJlc2VudGFibGUATm90IGEgdHR5AFBlcm1pc3Npb24gZGVuaWVkAE9wZXJhdGlvbiBub3QgcGVybWl0dGVkAE5vIHN1Y2ggZmlsZSBvciBkaXJlY3RvcnkATm8gc3VjaCBwcm9jZXNzAEZpbGUgZXhpc3RzAFZhbHVlIHRvbyBsYXJnZSBmb3IgZGF0YSB0eXBlAE5vIHNwYWNlIGxlZnQgb24gZGV2aWNlAE91dCBvZiBtZW1vcnkAUmVzb3VyY2UgYnVzeQBJbnRlcnJ1cHRlZCBzeXN0ZW0gY2FsbABSZXNvdXJjZSB0ZW1wb3JhcmlseSB1bmF2YWlsYWJsZQBJbnZhbGlkIHNlZWsAQ3Jvc3MtZGV2aWNlIGxpbmsAUmVhZC1vbmx5IGZpbGUgc3lzdGVtAERpcmVjdG9yeSBub3QgZW1wdHkAQ29ubmVjdGlvbiByZXNldCBieSBwZWVyAE9wZXJhdGlvbiB0aW1lZCBvdXQAQ29ubmVjdGlvbiByZWZ1c2VkAEhvc3QgaXMgZG93bgBIb3N0IGlzIHVucmVhY2hhYmxlAEFkZHJlc3MgaW4gdXNlAEJyb2tlbiBwaXBlAEkvTyBlcnJvcgBObyBzdWNoIGRldmljZSBvciBhZGRyZXNzAEJsb2NrIGRldmljZSByZXF1aXJlZABObyBzdWNoIGRldmljZQBOb3QgYSBkaXJlY3RvcnkASXMgYSBkaXJlY3RvcnkAVGV4dCBmaWxlIGJ1c3kARXhlYyBmb3JtYXQgZXJyb3IASW52YWxpZCBhcmd1bWVudABBcmd1bWVudCBsaXN0IHRvbyBsb25nAFN5bWJvbGljIGxpbmsgbG9vcABGaWxlbmFtZSB0b28gbG9uZwBUb28gbWFueSBvcGVuIGZpbGVzIGluIHN5c3RlbQBObyBmaWxlIGRlc2NyaXB0b3JzIGF2YWlsYWJsZQBCYWQgZmlsZSBkZXNjcmlwdG9yAE5vIGNoaWxkIHByb2Nlc3MAQmFkIGFkZHJlc3MARmlsZSB0b28gbGFyZ2UAVG9vIG1hbnkgbGlua3MATm8gbG9ja3MgYXZhaWxhYmxlAFJlc291cmNlIGRlYWRsb2NrIHdvdWxkIG9jY3VyAFN0YXRlIG5vdCByZWNvdmVyYWJsZQBQcmV2aW91cyBvd25lciBkaWVkAE9wZXJhdGlvbiBjYW5jZWxlZABGdW5jdGlvbiBub3QgaW1wbGVtZW50ZWQATm8gbWVzc2FnZSBvZiBkZXNpcmVkIHR5cGUASWRlbnRpZmllciByZW1vdmVkAERldmljZSBub3QgYSBzdHJlYW0ATm8gZGF0YSBhdmFpbGFibGUARGV2aWNlIHRpbWVvdXQAT3V0IG9mIHN0cmVhbXMgcmVzb3VyY2VzAExpbmsgaGFzIGJlZW4gc2V2ZXJlZABQcm90b2NvbCBlcnJvcgBCYWQgbWVzc2FnZQBGaWxlIGRlc2NyaXB0b3IgaW4gYmFkIHN0YXRlAE5vdCBhIHNvY2tldABEZXN0aW5hdGlvbiBhZGRyZXNzIHJlcXVpcmVkAE1lc3NhZ2UgdG9vIGxhcmdlAFByb3RvY29sIHdyb25nIHR5cGUgZm9yIHNvY2tldABQcm90b2NvbCBub3QgYXZhaWxhYmxlAFByb3RvY29sIG5vdCBzdXBwb3J0ZWQAU29ja2V0IHR5cGUgbm90IHN1cHBvcnRlZABOb3Qgc3VwcG9ydGVkAFByb3RvY29sIGZhbWlseSBub3Qgc3VwcG9ydGVkAEFkZHJlc3MgZmFtaWx5IG5vdCBzdXBwb3J0ZWQgYnkgcHJvdG9jb2wAQWRkcmVzcyBub3QgYXZhaWxhYmxlAE5ldHdvcmsgaXMgZG93bgBOZXR3b3JrIHVucmVhY2hhYmxlAENvbm5lY3Rpb24gcmVzZXQgYnkgbmV0d29yawBDb25uZWN0aW9uIGFib3J0ZWQATm8gYnVmZmVyIHNwYWNlIGF2YWlsYWJsZQBTb2NrZXQgaXMgY29ubmVjdGVkAFNvY2tldCBub3QgY29ubmVjdGVkAENhbm5vdCBzZW5kIGFmdGVyIHNvY2tldCBzaHV0ZG93bgBPcGVyYXRpb24gYWxyZWFkeSBpbiBwcm9ncmVzcwBPcGVyYXRpb24gaW4gcHJvZ3Jlc3MAU3RhbGUgZmlsZSBoYW5kbGUAUmVtb3RlIEkvTyBlcnJvcgBRdW90YSBleGNlZWRlZABObyBtZWRpdW0gZm91bmQAV3JvbmcgbWVkaXVtIHR5cGUATXVsdGlob3AgYXR0ZW1wdGVkAFJlcXVpcmVkIGtleSBub3QgYXZhaWxhYmxlAEtleSBoYXMgZXhwaXJlZABLZXkgaGFzIGJlZW4gcmV2b2tlZABLZXkgd2FzIHJlamVjdGVkIGJ5IHNlcnZpY2UAAAAAAAAAAAClAlsA8AG1BYwFJQGDBh0DlAT/AMcDMQMLBrwBjwF/A8oEKwDaBq8AQgNOA9wBDgQVAKEGDQGUAgsCOAZkArwC/wJdA+cECwfPAssF7wXbBeECHgZFAoUAggJsA28E8QDzAxgF2QDaA0wGVAJ7AZ0DvQQAAFEAFQK7ALMDbQD/AYUELwX5BDgAZQFGAZ8AtwaoAXMCUwEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAhBAAAAAAAAAAALwIAAAAAAAAAAAAAAAAAAAAAAAAAADUERwRWBAAAAAAAAAAAAAAAAAAAAACgBAAAAAAAAAAAAAAAAAAAAAAAAEYFYAVuBWEGAADPAQAAAAAAAAAAyQbpBvkGHgc5B0kHXgcAQeilBAuwAgUAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIAAAADAAAAaBQBAAAEAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAD/////CgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOgSAQAAIAAABQAAAAAAAAAAAAAABwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAAAAgAAAAwGQEAAAAAAAAAAAAAAAAAAgAAAAAAAAAAAAAAAAAAAP//////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgBMBACAbAQAAlAEPdGFyZ2V0X2ZlYXR1cmVzCCsLYnVsay1tZW1vcnkrD2J1bGstbWVtb3J5LW9wdCsWY2FsbC1pbmRpcmVjdC1vdmVybG9uZysKbXVsdGl2YWx1ZSsPbXV0YWJsZS1nbG9iYWxzKxNub250cmFwcGluZy1mcHRvaW50Kw9yZWZlcmVuY2UtdHlwZXMrCHNpZ24tZXh0');
}

function getBinarySync(file) {
  if (ArrayBuffer.isView(file)) {
    return file;
  }
  if (file == wasmBinaryFile && wasmBinary) {
    return new Uint8Array(wasmBinary);
  }
  if (readBinary) {
    return readBinary(file);
  }
  throw 'both async and sync fetching of the wasm failed';
}

async function getWasmBinary(binaryFile) {

  // Otherwise, getBinarySync should be able to get it synchronously
  return getBinarySync(binaryFile);
}

async function instantiateArrayBuffer(binaryFile, imports) {
  try {
    var binary = await getWasmBinary(binaryFile);
    var instance = await WebAssembly.instantiate(binary, imports);
    return instance;
  } catch (reason) {
    err(`failed to asynchronously prepare wasm: ${reason}`);

    // Warn on some common problems.
    if (isFileURI(wasmBinaryFile)) {
      err(`warning: Loading from a file URI (${wasmBinaryFile}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`);
    }
    abort(reason);
  }
}

async function instantiateAsync(binary, binaryFile, imports) {
  return instantiateArrayBuffer(binaryFile, imports);
}

function getWasmImports() {
  // prepare imports
  return {
    'env': wasmImports,
    'wasi_snapshot_preview1': wasmImports,
  }
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
async function createWasm() {
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    wasmExports = instance.exports;

    

    wasmMemory = wasmExports['memory'];
    
    assert(wasmMemory, 'memory not found in wasm exports');
    updateMemoryViews();

    assignWasmExports(wasmExports);
    removeRunDependency('wasm-instantiate');
    return wasmExports;
  }
  // wait for the pthread pool (if any)
  addRunDependency('wasm-instantiate');

  // Prefer streaming instantiation if available.
  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
  function receiveInstantiationResult(result) {
    // 'result' is a ResultObject object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above PTHREADS-enabled path.
    return receiveInstance(result['instance']);
  }

  var info = getWasmImports();

  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  if (Module['instantiateWasm']) {
    return new Promise((resolve, reject) => {
      try {
        Module['instantiateWasm'](info, (mod, inst) => {
          resolve(receiveInstance(mod, inst));
        });
      } catch(e) {
        err(`Module.instantiateWasm callback failed with error: ${e}`);
        reject(e);
      }
    });
  }

  wasmBinaryFile ??= findWasmBinary();
  var result = await instantiateAsync(wasmBinary, wasmBinaryFile, info);
  var exports = receiveInstantiationResult(result);
  return exports;
}

// end include: preamble.js

// Begin JS library code


  class ExitStatus {
      name = 'ExitStatus';
      constructor(status) {
        this.message = `Program terminated with exit(${status})`;
        this.status = status;
      }
    }

  var callRuntimeCallbacks = (callbacks) => {
      while (callbacks.length > 0) {
        // Pass the module as the first argument.
        callbacks.shift()(Module);
      }
    };
  var onPostRuns = [];
  var addOnPostRun = (cb) => onPostRuns.push(cb);

  var onPreRuns = [];
  var addOnPreRun = (cb) => onPreRuns.push(cb);

  /** @noinline */
  var base64Decode = (b64) => {
      if (ENVIRONMENT_IS_NODE) {
        var buf = Buffer.from(b64, 'base64');
        return new Uint8Array(buf.buffer, buf.byteOffset, buf.length);
      }
  
      assert(b64.length % 4 == 0);
      var b1, b2, i = 0, j = 0, bLength = b64.length;
      var output = new Uint8Array((bLength*3>>2) - (b64[bLength-2] == '=') - (b64[bLength-1] == '='));
      for (; i < bLength; i += 4, j += 3) {
        b1 = base64ReverseLookup[b64.charCodeAt(i+1)];
        b2 = base64ReverseLookup[b64.charCodeAt(i+2)];
        output[j] = base64ReverseLookup[b64.charCodeAt(i)] << 2 | b1 >> 4;
        output[j+1] = b1 << 4 | b2 >> 2;
        output[j+2] = b2 << 6 | base64ReverseLookup[b64.charCodeAt(i+3)];
      }
      return output;
    };


  
    /**
     * @param {number} ptr
     * @param {string} type
     */
  function getValue(ptr, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': return HEAP8[ptr];
      case 'i8': return HEAP8[ptr];
      case 'i16': return HEAP16[((ptr)>>1)];
      case 'i32': return HEAP32[((ptr)>>2)];
      case 'i64': return HEAP64[((ptr)>>3)];
      case 'float': return HEAPF32[((ptr)>>2)];
      case 'double': return HEAPF64[((ptr)>>3)];
      case '*': return HEAPU32[((ptr)>>2)];
      default: abort(`invalid type for getValue: ${type}`);
    }
  }

  var noExitRuntime = true;

  var ptrToString = (ptr) => {
      assert(typeof ptr === 'number');
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      ptr >>>= 0;
      return '0x' + ptr.toString(16).padStart(8, '0');
    };

  
    /**
     * @param {number} ptr
     * @param {number} value
     * @param {string} type
     */
  function setValue(ptr, value, type = 'i8') {
    if (type.endsWith('*')) type = '*';
    switch (type) {
      case 'i1': HEAP8[ptr] = value; break;
      case 'i8': HEAP8[ptr] = value; break;
      case 'i16': HEAP16[((ptr)>>1)] = value; break;
      case 'i32': HEAP32[((ptr)>>2)] = value; break;
      case 'i64': HEAP64[((ptr)>>3)] = BigInt(value); break;
      case 'float': HEAPF32[((ptr)>>2)] = value; break;
      case 'double': HEAPF64[((ptr)>>3)] = value; break;
      case '*': HEAPU32[((ptr)>>2)] = value; break;
      default: abort(`invalid type for setValue: ${type}`);
    }
  }

  var stackRestore = (val) => __emscripten_stack_restore(val);

  var stackSave = () => _emscripten_stack_get_current();

  var warnOnce = (text) => {
      warnOnce.shown ||= {};
      if (!warnOnce.shown[text]) {
        warnOnce.shown[text] = 1;
        if (ENVIRONMENT_IS_NODE) text = 'warning: ' + text;
        err(text);
      }
    };

  var __abort_js = () =>
      abort('native code called abort()');

  var getHeapMax = () =>
      // Stay one Wasm page short of 4GB: while e.g. Chrome is able to allocate
      // full 4GB Wasm memories, the size will wrap back to 0 bytes in Wasm side
      // for any code that deals with heap sizes, which would require special
      // casing all heap size related code to treat 0 specially.
      2147483648;
  
  var alignMemory = (size, alignment) => {
      assert(alignment, "alignment argument is required");
      return Math.ceil(size / alignment) * alignment;
    };
  
  var growMemory = (size) => {
      var oldHeapSize = wasmMemory.buffer.byteLength;
      var pages = ((size - oldHeapSize + 65535) / 65536) | 0;
      try {
        // round size grow request up to wasm page size (fixed 64KB per spec)
        wasmMemory.grow(pages); // .grow() takes a delta compared to the previous size
        updateMemoryViews();
        return 1 /*success*/;
      } catch(e) {
        err(`growMemory: Attempted to grow heap from ${oldHeapSize} bytes to ${size} bytes, but got error: ${e}`);
      }
      // implicit 0 return to save code size (caller will cast "undefined" into 0
      // anyhow)
    };
  var _emscripten_resize_heap = (requestedSize) => {
      var oldSize = HEAPU8.length;
      // With CAN_ADDRESS_2GB or MEMORY64, pointers are already unsigned.
      requestedSize >>>= 0;
      // With multithreaded builds, races can happen (another thread might increase the size
      // in between), so return a failure, and let the caller retry.
      assert(requestedSize > oldSize);
  
      // Memory resize rules:
      // 1.  Always increase heap size to at least the requested size, rounded up
      //     to next page multiple.
      // 2a. If MEMORY_GROWTH_LINEAR_STEP == -1, excessively resize the heap
      //     geometrically: increase the heap size according to
      //     MEMORY_GROWTH_GEOMETRIC_STEP factor (default +20%), At most
      //     overreserve by MEMORY_GROWTH_GEOMETRIC_CAP bytes (default 96MB).
      // 2b. If MEMORY_GROWTH_LINEAR_STEP != -1, excessively resize the heap
      //     linearly: increase the heap size by at least
      //     MEMORY_GROWTH_LINEAR_STEP bytes.
      // 3.  Max size for the heap is capped at 2048MB-WASM_PAGE_SIZE, or by
      //     MAXIMUM_MEMORY, or by ASAN limit, depending on which is smallest
      // 4.  If we were unable to allocate as much memory, it may be due to
      //     over-eager decision to excessively reserve due to (3) above.
      //     Hence if an allocation fails, cut down on the amount of excess
      //     growth, in an attempt to succeed to perform a smaller allocation.
  
      // A limit is set for how much we can grow. We should not exceed that
      // (the wasm binary specifies it, so if we tried, we'd fail anyhow).
      var maxHeapSize = getHeapMax();
      if (requestedSize > maxHeapSize) {
        err(`Cannot enlarge memory, requested ${requestedSize} bytes, but the limit is ${maxHeapSize} bytes!`);
        return false;
      }
  
      // Loop through potential heap size increases. If we attempt a too eager
      // reservation that fails, cut down on the attempted size and reserve a
      // smaller bump instead. (max 3 times, chosen somewhat arbitrarily)
      for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
        var overGrownHeapSize = oldSize * (1 + 0.2 / cutDown); // ensure geometric growth
        // but limit overreserving (default to capping at +96MB overgrowth at most)
        overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296 );
  
        var newSize = Math.min(maxHeapSize, alignMemory(Math.max(requestedSize, overGrownHeapSize), 65536));
  
        var replacement = growMemory(newSize);
        if (replacement) {
  
          return true;
        }
      }
      err(`Failed to grow the heap from ${oldSize} bytes to ${newSize} bytes, not enough memory!`);
      return false;
    };

  var UTF8Decoder = typeof TextDecoder != 'undefined' ? new TextDecoder() : undefined;
  
  var findStringEnd = (heapOrArray, idx, maxBytesToRead, ignoreNul) => {
      var maxIdx = idx + maxBytesToRead;
      if (ignoreNul) return maxIdx;
      // TextDecoder needs to know the byte length in advance, it doesn't stop on
      // null terminator by itself.
      // As a tiny code save trick, compare idx against maxIdx using a negation,
      // so that maxBytesToRead=undefined/NaN means Infinity.
      while (heapOrArray[idx] && !(idx >= maxIdx)) ++idx;
      return idx;
    };
  
    /**
     * Given a pointer 'idx' to a null-terminated UTF8-encoded string in the given
     * array that contains uint8 values, returns a copy of that string as a
     * Javascript String object.
     * heapOrArray is either a regular array, or a JavaScript typed array view.
     * @param {number=} idx
     * @param {number=} maxBytesToRead
     * @param {boolean=} ignoreNul - If true, the function will not stop on a NUL character.
     * @return {string}
     */
  var UTF8ArrayToString = (heapOrArray, idx = 0, maxBytesToRead, ignoreNul) => {
  
      var endPtr = findStringEnd(heapOrArray, idx, maxBytesToRead, ignoreNul);
  
      // When using conditional TextDecoder, skip it for short strings as the overhead of the native call is not worth it.
      if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
        return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr));
      }
      var str = '';
      // If building with TextDecoder, we have already computed the string length
      // above, so test loop end condition against that
      while (idx < endPtr) {
        // For UTF8 byte structure, see:
        // http://en.wikipedia.org/wiki/UTF-8#Description
        // https://www.ietf.org/rfc/rfc2279.txt
        // https://tools.ietf.org/html/rfc3629
        var u0 = heapOrArray[idx++];
        if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
        var u1 = heapOrArray[idx++] & 63;
        if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
        var u2 = heapOrArray[idx++] & 63;
        if ((u0 & 0xF0) == 0xE0) {
          u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
        } else {
          if ((u0 & 0xF8) != 0xF0) warnOnce('Invalid UTF-8 leading byte ' + ptrToString(u0) + ' encountered when deserializing a UTF-8 string in wasm memory to a JS string!');
          u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
        }
  
        if (u0 < 0x10000) {
          str += String.fromCharCode(u0);
        } else {
          var ch = u0 - 0x10000;
          str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
        }
      }
      return str;
    };
  
    /**
     * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
     * emscripten HEAP, returns a copy of that string as a Javascript String object.
     *
     * @param {number} ptr
     * @param {number=} maxBytesToRead - An optional length that specifies the
     *   maximum number of bytes to read. You can omit this parameter to scan the
     *   string until the first 0 byte. If maxBytesToRead is passed, and the string
     *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
     *   string will cut short at that byte index.
     * @param {boolean=} ignoreNul - If true, the function will not stop on a NUL character.
     * @return {string}
     */
  var UTF8ToString = (ptr, maxBytesToRead, ignoreNul) => {
      assert(typeof ptr == 'number', `UTF8ToString expects a number (got ${typeof ptr})`);
      return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead, ignoreNul) : '';
    };
  var SYSCALLS = {
  varargs:undefined,
  getStr(ptr) {
        var ret = UTF8ToString(ptr);
        return ret;
      },
  };
  var _fd_close = (fd) => {
      abort('fd_close called without SYSCALLS_REQUIRE_FILESYSTEM');
    };

  var INT53_MAX = 9007199254740992;
  
  var INT53_MIN = -9007199254740992;
  var bigintToI53Checked = (num) => (num < INT53_MIN || num > INT53_MAX) ? NaN : Number(num);
  function _fd_seek(fd, offset, whence, newOffset) {
    offset = bigintToI53Checked(offset);
  
  
      return 70;
    ;
  }

  var printCharBuffers = [null,[],[]];
  
  var printChar = (stream, curr) => {
      var buffer = printCharBuffers[stream];
      assert(buffer);
      if (curr === 0 || curr === 10) {
        (stream === 1 ? out : err)(UTF8ArrayToString(buffer));
        buffer.length = 0;
      } else {
        buffer.push(curr);
      }
    };
  
  var flush_NO_FILESYSTEM = () => {
      // flush anything remaining in the buffers during shutdown
      _fflush(0);
      if (printCharBuffers[1].length) printChar(1, 10);
      if (printCharBuffers[2].length) printChar(2, 10);
    };
  
  
  var _fd_write = (fd, iov, iovcnt, pnum) => {
      // hack to support printf in SYSCALLS_REQUIRE_FILESYSTEM=0
      var num = 0;
      for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAPU32[((iov)>>2)];
        var len = HEAPU32[(((iov)+(4))>>2)];
        iov += 8;
        for (var j = 0; j < len; j++) {
          printChar(fd, HEAPU8[ptr+j]);
        }
        num += len;
      }
      HEAPU32[((pnum)>>2)] = num;
      return 0;
    };

  var lengthBytesUTF8 = (str) => {
      var len = 0;
      for (var i = 0; i < str.length; ++i) {
        // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
        // unit, not a Unicode code point of the character! So decode
        // UTF16->UTF32->UTF8.
        // See http://unicode.org/faq/utf_bom.html#utf16-3
        var c = str.charCodeAt(i); // possibly a lead surrogate
        if (c <= 0x7F) {
          len++;
        } else if (c <= 0x7FF) {
          len += 2;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
          len += 4; ++i;
        } else {
          len += 3;
        }
      }
      return len;
    };

  var stringToUTF8Array = (str, heap, outIdx, maxBytesToWrite) => {
      assert(typeof str === 'string', `stringToUTF8Array expects a string (got ${typeof str})`);
      // Parameter maxBytesToWrite is not optional. Negative values, 0, null,
      // undefined and false each don't write out any bytes.
      if (!(maxBytesToWrite > 0))
        return 0;
  
      var startIdx = outIdx;
      var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
      for (var i = 0; i < str.length; ++i) {
        // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description
        // and https://www.ietf.org/rfc/rfc2279.txt
        // and https://tools.ietf.org/html/rfc3629
        var u = str.codePointAt(i);
        if (u <= 0x7F) {
          if (outIdx >= endIdx) break;
          heap[outIdx++] = u;
        } else if (u <= 0x7FF) {
          if (outIdx + 1 >= endIdx) break;
          heap[outIdx++] = 0xC0 | (u >> 6);
          heap[outIdx++] = 0x80 | (u & 63);
        } else if (u <= 0xFFFF) {
          if (outIdx + 2 >= endIdx) break;
          heap[outIdx++] = 0xE0 | (u >> 12);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
        } else {
          if (outIdx + 3 >= endIdx) break;
          if (u > 0x10FFFF) warnOnce('Invalid Unicode code point ' + ptrToString(u) + ' encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).');
          heap[outIdx++] = 0xF0 | (u >> 18);
          heap[outIdx++] = 0x80 | ((u >> 12) & 63);
          heap[outIdx++] = 0x80 | ((u >> 6) & 63);
          heap[outIdx++] = 0x80 | (u & 63);
          // Gotcha: if codePoint is over 0xFFFF, it is represented as a surrogate pair in UTF-16.
          // We need to manually skip over the second code unit for correct iteration.
          i++;
        }
      }
      // Null-terminate the pointer to the buffer.
      heap[outIdx] = 0;
      return outIdx - startIdx;
    };
  var stringToUTF8 = (str, outPtr, maxBytesToWrite) => {
      assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
      return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
    };

    // Precreate a reverse lookup table from chars
    // "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" back to
    // bytes to make decoding fast.
    for (var base64ReverseLookup = new Uint8Array(123/*'z'+1*/), i = 25; i >= 0; --i) {
      base64ReverseLookup[48+i] = 52+i; // '0-9'
      base64ReverseLookup[65+i] = i; // 'A-Z'
      base64ReverseLookup[97+i] = 26+i; // 'a-z'
    }
    base64ReverseLookup[43] = 62; // '+'
    base64ReverseLookup[47] = 63; // '/'
  ;
// End JS library code

// include: postlibrary.js
// This file is included after the automatically-generated JS library code
// but before the wasm module is created.

{

  // Begin ATMODULES hooks
  if (Module['noExitRuntime']) noExitRuntime = Module['noExitRuntime'];
if (Module['print']) out = Module['print'];
if (Module['printErr']) err = Module['printErr'];
if (Module['wasmBinary']) wasmBinary = Module['wasmBinary'];

Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;

  // End ATMODULES hooks

  checkIncomingModuleAPI();

  if (Module['arguments']) arguments_ = Module['arguments'];
  if (Module['thisProgram']) thisProgram = Module['thisProgram'];

  // Assertions on removed incoming Module JS APIs.
  assert(typeof Module['memoryInitializerPrefixURL'] == 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['pthreadMainPrefixURL'] == 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['cdInitializerPrefixURL'] == 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['filePackagePrefixURL'] == 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
  assert(typeof Module['read'] == 'undefined', 'Module.read option was removed');
  assert(typeof Module['readAsync'] == 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
  assert(typeof Module['readBinary'] == 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
  assert(typeof Module['setWindowTitle'] == 'undefined', 'Module.setWindowTitle option was removed (modify emscripten_set_window_title in JS)');
  assert(typeof Module['TOTAL_MEMORY'] == 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
  assert(typeof Module['ENVIRONMENT'] == 'undefined', 'Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node)');
  assert(typeof Module['STACK_SIZE'] == 'undefined', 'STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time')
  // If memory is defined in wasm, the user can't provide it, or set INITIAL_MEMORY
  assert(typeof Module['wasmMemory'] == 'undefined', 'Use of `wasmMemory` detected.  Use -sIMPORTED_MEMORY to define wasmMemory externally');
  assert(typeof Module['INITIAL_MEMORY'] == 'undefined', 'Detected runtime INITIAL_MEMORY setting.  Use -sIMPORTED_MEMORY to define wasmMemory dynamically');

}

// Begin runtime exports
  Module['stringToUTF8'] = stringToUTF8;
  Module['lengthBytesUTF8'] = lengthBytesUTF8;
  var missingLibrarySymbols = [
  'writeI53ToI64',
  'writeI53ToI64Clamped',
  'writeI53ToI64Signaling',
  'writeI53ToU64Clamped',
  'writeI53ToU64Signaling',
  'readI53FromI64',
  'readI53FromU64',
  'convertI32PairToI53',
  'convertI32PairToI53Checked',
  'convertU32PairToI53',
  'stackAlloc',
  'getTempRet0',
  'setTempRet0',
  'zeroMemory',
  'exitJS',
  'withStackSave',
  'strError',
  'inetPton4',
  'inetNtop4',
  'inetPton6',
  'inetNtop6',
  'readSockaddr',
  'writeSockaddr',
  'readEmAsmArgs',
  'jstoi_q',
  'getExecutableName',
  'autoResumeAudioContext',
  'getDynCaller',
  'dynCall',
  'handleException',
  'keepRuntimeAlive',
  'runtimeKeepalivePush',
  'runtimeKeepalivePop',
  'callUserCallback',
  'maybeExit',
  'asmjsMangle',
  'asyncLoad',
  'mmapAlloc',
  'HandleAllocator',
  'getNativeTypeSize',
  'getUniqueRunDependency',
  'addOnInit',
  'addOnPostCtor',
  'addOnPreMain',
  'addOnExit',
  'STACK_SIZE',
  'STACK_ALIGN',
  'POINTER_SIZE',
  'ASSERTIONS',
  'ccall',
  'cwrap',
  'convertJsFunctionToWasm',
  'getEmptyTableSlot',
  'updateTableMap',
  'getFunctionAddress',
  'addFunction',
  'removeFunction',
  'intArrayFromString',
  'intArrayToString',
  'AsciiToString',
  'stringToAscii',
  'UTF16ToString',
  'stringToUTF16',
  'lengthBytesUTF16',
  'UTF32ToString',
  'stringToUTF32',
  'lengthBytesUTF32',
  'stringToNewUTF8',
  'stringToUTF8OnStack',
  'writeArrayToMemory',
  'registerKeyEventCallback',
  'maybeCStringToJsString',
  'findEventTarget',
  'getBoundingClientRect',
  'fillMouseEventData',
  'registerMouseEventCallback',
  'registerWheelEventCallback',
  'registerUiEventCallback',
  'registerFocusEventCallback',
  'fillDeviceOrientationEventData',
  'registerDeviceOrientationEventCallback',
  'fillDeviceMotionEventData',
  'registerDeviceMotionEventCallback',
  'screenOrientation',
  'fillOrientationChangeEventData',
  'registerOrientationChangeEventCallback',
  'fillFullscreenChangeEventData',
  'registerFullscreenChangeEventCallback',
  'JSEvents_requestFullscreen',
  'JSEvents_resizeCanvasForFullscreen',
  'registerRestoreOldStyle',
  'hideEverythingExceptGivenElement',
  'restoreHiddenElements',
  'setLetterbox',
  'softFullscreenResizeWebGLRenderTarget',
  'doRequestFullscreen',
  'fillPointerlockChangeEventData',
  'registerPointerlockChangeEventCallback',
  'registerPointerlockErrorEventCallback',
  'requestPointerLock',
  'fillVisibilityChangeEventData',
  'registerVisibilityChangeEventCallback',
  'registerTouchEventCallback',
  'fillGamepadEventData',
  'registerGamepadEventCallback',
  'registerBeforeUnloadEventCallback',
  'fillBatteryEventData',
  'registerBatteryEventCallback',
  'setCanvasElementSize',
  'getCanvasElementSize',
  'jsStackTrace',
  'getCallstack',
  'convertPCtoSourceLocation',
  'getEnvStrings',
  'checkWasiClock',
  'wasiRightsToMuslOFlags',
  'wasiOFlagsToMuslOFlags',
  'initRandomFill',
  'randomFill',
  'safeSetTimeout',
  'setImmediateWrapped',
  'safeRequestAnimationFrame',
  'clearImmediateWrapped',
  'registerPostMainLoop',
  'registerPreMainLoop',
  'getPromise',
  'makePromise',
  'idsToPromises',
  'makePromiseCallback',
  'ExceptionInfo',
  'findMatchingCatch',
  'Browser_asyncPrepareDataCounter',
  'isLeapYear',
  'ydayFromDate',
  'arraySum',
  'addDays',
  'getSocketFromFD',
  'getSocketAddress',
  'FS_createPreloadedFile',
  'FS_modeStringToFlags',
  'FS_getMode',
  'FS_stdin_getChar',
  'FS_mkdirTree',
  '_setNetworkCallback',
  'heapObjectForWebGLType',
  'toTypedArrayIndex',
  'webgl_enable_ANGLE_instanced_arrays',
  'webgl_enable_OES_vertex_array_object',
  'webgl_enable_WEBGL_draw_buffers',
  'webgl_enable_WEBGL_multi_draw',
  'webgl_enable_EXT_polygon_offset_clamp',
  'webgl_enable_EXT_clip_control',
  'webgl_enable_WEBGL_polygon_mode',
  'emscriptenWebGLGet',
  'computeUnpackAlignedImageSize',
  'colorChannelsInGlTextureFormat',
  'emscriptenWebGLGetTexPixelData',
  'emscriptenWebGLGetUniform',
  'webglGetUniformLocation',
  'webglPrepareUniformLocationsBeforeFirstUse',
  'webglGetLeftBracePos',
  'emscriptenWebGLGetVertexAttrib',
  '__glGetActiveAttribOrUniform',
  'writeGLArray',
  'registerWebGlEventCallback',
  'runAndAbortIfError',
  'ALLOC_NORMAL',
  'ALLOC_STACK',
  'allocate',
  'writeStringToMemory',
  'writeAsciiToMemory',
  'demangle',
  'stackTrace',
];
missingLibrarySymbols.forEach(missingLibrarySymbol)

  var unexportedSymbols = [
  'run',
  'addRunDependency',
  'removeRunDependency',
  'out',
  'err',
  'callMain',
  'abort',
  'wasmMemory',
  'wasmExports',
  'HEAPF32',
  'HEAPF64',
  'HEAP8',
  'HEAPU8',
  'HEAP16',
  'HEAPU16',
  'HEAP32',
  'HEAPU32',
  'HEAP64',
  'HEAPU64',
  'writeStackCookie',
  'checkStackCookie',
  'INT53_MAX',
  'INT53_MIN',
  'bigintToI53Checked',
  'stackSave',
  'stackRestore',
  'ptrToString',
  'getHeapMax',
  'growMemory',
  'ENV',
  'ERRNO_CODES',
  'DNS',
  'Protocols',
  'Sockets',
  'timers',
  'warnOnce',
  'readEmAsmArgsArray',
  'alignMemory',
  'wasmTable',
  'noExitRuntime',
  'addOnPreRun',
  'addOnPostRun',
  'freeTableIndexes',
  'functionsInTableMap',
  'setValue',
  'getValue',
  'PATH',
  'PATH_FS',
  'UTF8Decoder',
  'UTF8ArrayToString',
  'UTF8ToString',
  'stringToUTF8Array',
  'UTF16Decoder',
  'JSEvents',
  'specialHTMLTargets',
  'findCanvasEventTarget',
  'currentFullscreenStrategy',
  'restoreOldWindowedStyle',
  'UNWIND_CACHE',
  'ExitStatus',
  'flush_NO_FILESYSTEM',
  'emSetImmediate',
  'emClearImmediate_deps',
  'emClearImmediate',
  'promiseMap',
  'uncaughtExceptionCount',
  'exceptionLast',
  'exceptionCaught',
  'Browser',
  'requestFullscreen',
  'requestFullScreen',
  'setCanvasSize',
  'getUserMedia',
  'createContext',
  'getPreloadedImageData__data',
  'wget',
  'MONTH_DAYS_REGULAR',
  'MONTH_DAYS_LEAP',
  'MONTH_DAYS_REGULAR_CUMULATIVE',
  'MONTH_DAYS_LEAP_CUMULATIVE',
  'base64Decode',
  'SYSCALLS',
  'preloadPlugins',
  'FS_stdin_getChar_buffer',
  'FS_unlink',
  'FS_createPath',
  'FS_createDevice',
  'FS_readFile',
  'FS',
  'FS_root',
  'FS_mounts',
  'FS_devices',
  'FS_streams',
  'FS_nextInode',
  'FS_nameTable',
  'FS_currentPath',
  'FS_initialized',
  'FS_ignorePermissions',
  'FS_filesystems',
  'FS_syncFSRequests',
  'FS_readFiles',
  'FS_lookupPath',
  'FS_getPath',
  'FS_hashName',
  'FS_hashAddNode',
  'FS_hashRemoveNode',
  'FS_lookupNode',
  'FS_createNode',
  'FS_destroyNode',
  'FS_isRoot',
  'FS_isMountpoint',
  'FS_isFile',
  'FS_isDir',
  'FS_isLink',
  'FS_isChrdev',
  'FS_isBlkdev',
  'FS_isFIFO',
  'FS_isSocket',
  'FS_flagsToPermissionString',
  'FS_nodePermissions',
  'FS_mayLookup',
  'FS_mayCreate',
  'FS_mayDelete',
  'FS_mayOpen',
  'FS_checkOpExists',
  'FS_nextfd',
  'FS_getStreamChecked',
  'FS_getStream',
  'FS_createStream',
  'FS_closeStream',
  'FS_dupStream',
  'FS_doSetAttr',
  'FS_chrdev_stream_ops',
  'FS_major',
  'FS_minor',
  'FS_makedev',
  'FS_registerDevice',
  'FS_getDevice',
  'FS_getMounts',
  'FS_syncfs',
  'FS_mount',
  'FS_unmount',
  'FS_lookup',
  'FS_mknod',
  'FS_statfs',
  'FS_statfsStream',
  'FS_statfsNode',
  'FS_create',
  'FS_mkdir',
  'FS_mkdev',
  'FS_symlink',
  'FS_rename',
  'FS_rmdir',
  'FS_readdir',
  'FS_readlink',
  'FS_stat',
  'FS_fstat',
  'FS_lstat',
  'FS_doChmod',
  'FS_chmod',
  'FS_lchmod',
  'FS_fchmod',
  'FS_doChown',
  'FS_chown',
  'FS_lchown',
  'FS_fchown',
  'FS_doTruncate',
  'FS_truncate',
  'FS_ftruncate',
  'FS_utime',
  'FS_open',
  'FS_close',
  'FS_isClosed',
  'FS_llseek',
  'FS_read',
  'FS_write',
  'FS_mmap',
  'FS_msync',
  'FS_ioctl',
  'FS_writeFile',
  'FS_cwd',
  'FS_chdir',
  'FS_createDefaultDirectories',
  'FS_createDefaultDevices',
  'FS_createSpecialDirectories',
  'FS_createStandardStreams',
  'FS_staticInit',
  'FS_init',
  'FS_quit',
  'FS_findObject',
  'FS_analyzePath',
  'FS_createFile',
  'FS_createDataFile',
  'FS_forceLoadFile',
  'FS_createLazyFile',
  'FS_absolutePath',
  'FS_createFolder',
  'FS_createLink',
  'FS_joinPath',
  'FS_mmapAlloc',
  'FS_standardizePath',
  'MEMFS',
  'TTY',
  'PIPEFS',
  'SOCKFS',
  'tempFixedLengthArray',
  'miniTempWebGLFloatBuffers',
  'miniTempWebGLIntBuffers',
  'GL',
  'AL',
  'GLUT',
  'EGL',
  'GLEW',
  'IDBStore',
  'SDL',
  'SDL_gfx',
  'allocateUTF8',
  'allocateUTF8OnStack',
  'print',
  'printErr',
  'jstoi_s',
];
unexportedSymbols.forEach(unexportedRuntimeSymbol);

  // End runtime exports
  // Begin JS library exports
  // End JS library exports

// end include: postlibrary.js

function checkIncomingModuleAPI() {
  ignoredModuleProp('fetchSettings');
}

// Imports from the Wasm binary.
var _fflush = makeInvalidEarlyAccess('_fflush');
var _malloc = Module['_malloc'] = makeInvalidEarlyAccess('_malloc');
var _thread_demo = Module['_thread_demo'] = makeInvalidEarlyAccess('_thread_demo');
var _demo_fcfs = Module['_demo_fcfs'] = makeInvalidEarlyAccess('_demo_fcfs');
var _demo_sjf = Module['_demo_sjf'] = makeInvalidEarlyAccess('_demo_sjf');
var _demo_priority = Module['_demo_priority'] = makeInvalidEarlyAccess('_demo_priority');
var _demo_round_robin = Module['_demo_round_robin'] = makeInvalidEarlyAccess('_demo_round_robin');
var _run = Module['_run'] = makeInvalidEarlyAccess('_run');
var _default_demo = Module['_default_demo'] = makeInvalidEarlyAccess('_default_demo');
var _strerror = makeInvalidEarlyAccess('_strerror');
var _emscripten_stack_get_end = makeInvalidEarlyAccess('_emscripten_stack_get_end');
var _emscripten_stack_get_base = makeInvalidEarlyAccess('_emscripten_stack_get_base');
var _free = Module['_free'] = makeInvalidEarlyAccess('_free');
var _emscripten_stack_init = makeInvalidEarlyAccess('_emscripten_stack_init');
var _emscripten_stack_get_free = makeInvalidEarlyAccess('_emscripten_stack_get_free');
var __emscripten_stack_restore = makeInvalidEarlyAccess('__emscripten_stack_restore');
var __emscripten_stack_alloc = makeInvalidEarlyAccess('__emscripten_stack_alloc');
var _emscripten_stack_get_current = makeInvalidEarlyAccess('_emscripten_stack_get_current');

function assignWasmExports(wasmExports) {
  _fflush = createExportWrapper('fflush', 1);
  Module['_malloc'] = _malloc = createExportWrapper('malloc', 1);
  Module['_thread_demo'] = _thread_demo = createExportWrapper('thread_demo', 1);
  Module['_demo_fcfs'] = _demo_fcfs = createExportWrapper('demo_fcfs', 0);
  Module['_demo_sjf'] = _demo_sjf = createExportWrapper('demo_sjf', 0);
  Module['_demo_priority'] = _demo_priority = createExportWrapper('demo_priority', 0);
  Module['_demo_round_robin'] = _demo_round_robin = createExportWrapper('demo_round_robin', 0);
  Module['_run'] = _run = createExportWrapper('run', 1);
  Module['_default_demo'] = _default_demo = createExportWrapper('default_demo', 0);
  _strerror = createExportWrapper('strerror', 1);
  _emscripten_stack_get_end = wasmExports['emscripten_stack_get_end'];
  _emscripten_stack_get_base = wasmExports['emscripten_stack_get_base'];
  Module['_free'] = _free = createExportWrapper('free', 1);
  _emscripten_stack_init = wasmExports['emscripten_stack_init'];
  _emscripten_stack_get_free = wasmExports['emscripten_stack_get_free'];
  __emscripten_stack_restore = wasmExports['_emscripten_stack_restore'];
  __emscripten_stack_alloc = wasmExports['_emscripten_stack_alloc'];
  _emscripten_stack_get_current = wasmExports['emscripten_stack_get_current'];
}
var wasmImports = {
  /** @export */
  _abort_js: __abort_js,
  /** @export */
  emscripten_resize_heap: _emscripten_resize_heap,
  /** @export */
  fd_close: _fd_close,
  /** @export */
  fd_seek: _fd_seek,
  /** @export */
  fd_write: _fd_write
};
var wasmExports = await createWasm();


// include: postamble.js
// === Auto-generated postamble setup entry stuff ===

var calledRun;

function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  _emscripten_stack_init();
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  writeStackCookie();
}

function run() {

  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  stackCheckInit();

  preRun();

  // a preRun added a dependency, run will be called later
  if (runDependencies > 0) {
    dependenciesFulfilled = run;
    return;
  }

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
    assert(!calledRun);
    calledRun = true;
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

    readyPromiseResolve?.(Module);
    Module['onRuntimeInitialized']?.();
    consumedModuleProp('onRuntimeInitialized');

    assert(!Module['_main'], 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');

    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(() => {
      setTimeout(() => Module['setStatus'](''), 1);
      doRun();
    }, 1);
  } else
  {
    doRun();
  }
  checkStackCookie();
}

function checkUnflushedContent() {
  // Compiler settings do not allow exiting the runtime, so flushing
  // the streams is not possible. but in ASSERTIONS mode we check
  // if there was something to flush, and if so tell the user they
  // should request that the runtime be exitable.
  // Normally we would not even include flush() at all, but in ASSERTIONS
  // builds we do so just for this check, and here we see if there is any
  // content to flush, that is, we check if there would have been
  // something a non-ASSERTIONS build would have not seen.
  // How we flush the streams depends on whether we are in SYSCALLS_REQUIRE_FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var oldOut = out;
  var oldErr = err;
  var has = false;
  out = err = (x) => {
    has = true;
  }
  try { // it doesn't matter if it fails
    flush_NO_FILESYSTEM();
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the Emscripten FAQ), or make sure to emit a newline when you printf etc.');
    warnOnce('(this may also be due to not including full filesystem support - try building with -sFORCE_FILESYSTEM)');
  }
}

function preInit() {
  if (Module['preInit']) {
    if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
    while (Module['preInit'].length > 0) {
      Module['preInit'].shift()();
    }
  }
  consumedModuleProp('preInit');
}

preInit();
run();

// end include: postamble.js

// include: postamble_modularize.js
// In MODULARIZE mode we wrap the generated code in a factory function
// and return either the Module itself, or a promise of the module.
//
// We assign to the `moduleRtn` global here and configure closure to see
// this as and extern so it won't get minified.

if (runtimeInitialized)  {
  moduleRtn = Module;
} else {
  // Set up the promise that indicates the Module is initialized
  moduleRtn = new Promise((resolve, reject) => {
    readyPromiseResolve = resolve;
    readyPromiseReject = reject;
  });
}

// Assertion for attempting to access module properties on the incoming
// moduleArg.  In the past we used this object as the prototype of the module
// and assigned properties to it, but now we return a distinct object.  This
// keeps the instance private until it is ready (i.e the promise has been
// resolved).
for (const prop of Object.keys(Module)) {
  if (!(prop in moduleArg)) {
    Object.defineProperty(moduleArg, prop, {
      configurable: true,
      get() {
        abort(`Access to module property ('${prop}') is no longer possible via the module constructor argument; Instead, use the result of the module constructor.`)
      }
    });
  }
}
// end include: postamble_modularize.js



    return moduleRtn;
  };
})();

// Export using a UMD style export, or ES6 exports if selected
if (typeof exports === 'object' && typeof module === 'object') {
  module.exports = LosuThread;
  // This default export looks redundant, but it allows TS to import this
  // commonjs style module.
  module.exports.default = LosuThread;
} else if (typeof define === 'function' && define['amd'])
  define([], () => LosuThread);

