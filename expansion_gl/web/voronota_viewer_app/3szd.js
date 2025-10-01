
  var Module = typeof Module != 'undefined' ? Module : {};

  Module['expectedDataFileDownloads'] ??= 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string';
    async function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = './3szd.data';
      var REMOTE_PACKAGE_BASE = '3szd.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile']?.(REMOTE_PACKAGE_BASE, '') ?? REMOTE_PACKAGE_BASE;
      var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      async function fetchRemotePackage(packageName, packageSize) {
        if (isNode) {
          var fsPromises = require('fs/promises');
          var contents = await fsPromises.readFile(packageName);
          return contents.buffer;
        }
        Module['dataFileDownloads'] ??= {};
        try {
          var response = await fetch(packageName);
        } catch (e) {
          throw new Error(`Network Error: ${packageName}`, {e});
        }
        if (!response.ok) {
          throw new Error(`${response.status}: ${response.url}`);
        }

        const chunks = [];
        const headers = response.headers;
        const total = Number(headers.get('Content-Length') ?? packageSize);
        let loaded = 0;

        Module['setStatus']?.('Downloading data...');
        const reader = response.body.getReader();

        while (1) {
          var {done, value} = await reader.read();
          if (done) break;
          chunks.push(value);
          loaded += value.length;
          Module['dataFileDownloads'][packageName] = {loaded, total};

          let totalLoaded = 0;
          let totalSize = 0;

          for (const download of Object.values(Module['dataFileDownloads'])) {
            totalLoaded += download.loaded;
            totalSize += download.total;
          }

          Module['setStatus']?.(`Downloading data... (${totalLoaded}/${totalSize})`);
        }

        const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
        let offset = 0;
        for (const chunk of chunks) {
          packageData.set(chunk, offset);
          offset += chunk.length;
        }
        return packageData.buffer;
      }

      var fetchPromise;
      var fetched = Module['getPreloadedPackage']?.(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);

      if (!fetched) {
        // Note that we don't use await here because we want to execute the
        // the rest of this function immediately.
        fetchPromise = fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);
      }

    async function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw new Error(msg);
      }

      async function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        var compressedData = {"data":null,"cachedOffset":160204,"cachedIndexes":[-1,-1],"cachedChunks":[null,null],"offsets":[0,748,1516,2446,3561,4653,5777,6902,8016,9112,10251,11332,12427,13541,14614,15719,16794,17908,19009,20126,21223,22320,23436,24511,25609,26698,27794,28910,30004,31056,32119,33207,34279,35396,36511,37596,38677,39796,40840,41884,42970,44059,45150,46247,47354,48440,49517,50641,51740,52818,53889,54958,56015,57082,58144,59226,60332,61431,62479,63551,64618,65696,66760,67832,68882,69950,71050,72144,73205,74297,75394,76489,77579,78645,79719,80787,81841,82943,84054,85144,86232,87335,88411,89492,90557,91639,92730,93840,94918,95969,97026,98110,99172,100248,101315,102355,103426,104510,105613,106703,107805,108893,109988,111080,112198,113264,114365,115498,116597,117698,118807,119902,120986,122088,123200,124285,125393,126481,127614,128740,129836,130975,132115,133251,134406,135045,135496,135931,136358,136808,137252,137690,138140,138580,139022,139458,139899,140333,140779,141216,141643,142085,142509,142959,143385,143842,144277,144718,145161,145605,146063,146496,146950,147402,147844,148299,148743,149196,149643,150072,150526,150968,151421,151852,152309,152757,153207,153645,154083,154549,154988,155451,155881,156351,156787,157236,157661,158119,158562,158993,159443,159916],"sizes":[748,768,930,1115,1092,1124,1125,1114,1096,1139,1081,1095,1114,1073,1105,1075,1114,1101,1117,1097,1097,1116,1075,1098,1089,1096,1116,1094,1052,1063,1088,1072,1117,1115,1085,1081,1119,1044,1044,1086,1089,1091,1097,1107,1086,1077,1124,1099,1078,1071,1069,1057,1067,1062,1082,1106,1099,1048,1072,1067,1078,1064,1072,1050,1068,1100,1094,1061,1092,1097,1095,1090,1066,1074,1068,1054,1102,1111,1090,1088,1103,1076,1081,1065,1082,1091,1110,1078,1051,1057,1084,1062,1076,1067,1040,1071,1084,1103,1090,1102,1088,1095,1092,1118,1066,1101,1133,1099,1101,1109,1095,1084,1102,1112,1085,1108,1088,1133,1126,1096,1139,1140,1136,1155,639,451,435,427,450,444,438,450,440,442,436,441,434,446,437,427,442,424,450,426,457,435,441,443,444,458,433,454,452,442,455,444,453,447,429,454,442,453,431,457,448,450,438,438,466,439,463,430,470,436,449,425,458,443,431,450,473,288],"successes":[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]}
;
            compressedData['data'] = byteArray;
            assert(typeof Module['LZ4'] === 'object', 'LZ4 not present - was your app build with -sLZ4?');
            Module['LZ4'].loadPackage({ 'metadata': metadata, 'compressedData': compressedData }, false);
            Module['removeRunDependency']('datafile_./3szd.data');
      }
      Module['addRunDependency']('datafile_./3szd.data');

      Module['preloadResults'] ??= {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (!fetched) {
        fetched = await fetchPromise;
      }
      processPackageData(fetched);

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      (Module['preRun'] ??= []).push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/3szd.pdb", "start": 0, "end": 371871}], "remote_package_size": 164300});

  })();
