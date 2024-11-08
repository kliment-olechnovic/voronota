
  var Module = typeof Module !== 'undefined' ? Module : {};

  if (!Module.expectedDataFileDownloads) {
    Module.expectedDataFileDownloads = 0;
  }

  Module.expectedDataFileDownloads++;
  (function() {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    if (Module['ENVIRONMENT_IS_PTHREAD'] || Module['$ww']) return;
    var loadPackage = function(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = './ensemble.data';
      var REMOTE_PACKAGE_BASE = 'ensemble.data';
      if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
        Module['locateFile'] = Module['locateFilePackage'];
        err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
      }
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        if (typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string') {
          require('fs').readFile(packageName, function(err, contents) {
            if (err) {
              errback(err);
            } else {
              callback(contents.buffer);
            }
          });
          return;
        }
        var xhr = new XMLHttpRequest();
        xhr.open('GET', packageName, true);
        xhr.responseType = 'arraybuffer';
        xhr.onprogress = function(event) {
          var url = packageName;
          var size = packageSize;
          if (event.total) size = event.total;
          if (event.loaded) {
            if (!xhr.addedTotal) {
              xhr.addedTotal = true;
              if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
              Module.dataFileDownloads[url] = {
                loaded: event.loaded,
                total: size
              };
            } else {
              Module.dataFileDownloads[url].loaded = event.loaded;
            }
            var total = 0;
            var loaded = 0;
            var num = 0;
            for (var download in Module.dataFileDownloads) {
            var data = Module.dataFileDownloads[download];
              total += data.total;
              loaded += data.loaded;
              num++;
            }
            total = Math.ceil(total * Module.expectedDataFileDownloads/num);
            if (Module['setStatus']) Module['setStatus'](`Downloading data... (${loaded}/${total})`);
          } else if (!Module.dataFileDownloads) {
            if (Module['setStatus']) Module['setStatus']('Downloading data...');
          }
        };
        xhr.onerror = function(event) {
          throw new Error("NetworkError for: " + packageName);
        }
        xhr.onload = function(event) {
          if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            var packageData = xhr.response;
            callback(packageData);
          } else {
            throw new Error(xhr.statusText + " : " + xhr.responseURL);
          }
        };
        xhr.send(null);
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS() {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        var compressedData = {"data":null,"cachedOffset":471987,"cachedIndexes":[-1,-1],"cachedChunks":[null,null],"offsets":[0,2048,4096,6144,8192,10240,12288,14336,16384,18432,20480,22528,24576,26624,28542,30390,32228,33869,35917,37965,40013,42061,44109,46157,48205,50253,52301,54349,56397,58445,60493,62541,64580,66393,68286,69992,72033,74081,76129,78177,80225,82273,84321,86369,88417,90465,92513,94561,96609,98657,100705,102478,104370,106174,108109,110165,112213,114261,116309,118357,120405,122453,124501,126558,128606,130654,132702,134759,136807,138612,140463,142249,144046,146094,148142,150190,152238,154286,156334,158382,160430,162478,164526,166574,168622,170670,172718,174698,176516,178460,180131,182179,184227,186275,188323,190371,192419,194467,196515,198563,200611,202659,204707,206755,208803,210851,212636,214504,216231,218231,220279,222327,224375,226423,228471,230519,232567,234615,236663,238711,240759,242807,244855,246903,248700,250604,252386,254326,256374,258422,260470,262518,264566,266614,268662,270710,272758,274806,276854,278902,280950,282998,284826,286727,288523,290389,292437,294485,296533,298581,300629,302677,304725,306773,308821,310869,312917,314965,317013,319061,320905,322749,324595,326257,328305,330353,332410,334458,336506,338554,340602,342650,344698,346746,348794,350842,352890,354938,356945,358786,360666,362370,364418,366466,368514,370562,372610,374658,376706,378754,380802,382850,384898,386946,388994,391042,393090,394907,396797,398532,400551,402599,404647,406695,408743,410791,412839,414887,416935,418983,421031,423079,425127,427175,429223,431005,432912,434627,436627,438675,440723,442771,444828,446876,448924,450972,453020,455068,457116,459164,461212,463260,465308,467094,469046,470844],"sizes":[2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1918,1848,1838,1641,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2039,1813,1893,1706,2041,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1773,1892,1804,1935,2056,2048,2048,2048,2048,2048,2048,2048,2057,2048,2048,2048,2057,2048,1805,1851,1786,1797,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1980,1818,1944,1671,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1785,1868,1727,2000,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1797,1904,1782,1940,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1828,1901,1796,1866,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1844,1844,1846,1662,2048,2048,2057,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2007,1841,1880,1704,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1817,1890,1735,2019,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1782,1907,1715,2000,2048,2048,2048,2057,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,1786,1952,1798,1143],"successes":[0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1]}
;
            compressedData['data'] = byteArray;
            assert(typeof Module['LZ4'] === 'object', 'LZ4 not present - was your app build with -sLZ4?');
            Module['LZ4'].loadPackage({ 'metadata': metadata, 'compressedData': compressedData }, false);
            Module['removeRunDependency']('datafile_./ensemble.data');
      };
      Module['addRunDependency']('datafile_./ensemble.data');

      if (!Module.preloadResults) Module.preloadResults = {};

      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS();
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/ensemble.vses", "start": 0, "end": 482574}], "remote_package_size": 476083});

  })();
