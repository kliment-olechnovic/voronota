
var Module = typeof Module !== 'undefined' ? Module : {};

if (!Module.expectedDataFileDownloads) {
  Module.expectedDataFileDownloads = 0;
  Module.finishedDataFileDownloads = 0;
}
Module.expectedDataFileDownloads++;
(function() {
 var loadPackage = function(metadata) {

    var PACKAGE_PATH;
    if (typeof window === 'object') {
      PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
    } else if (typeof location !== 'undefined') {
      // worker
      PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
    } else {
      throw 'using preloaded data can only be done on a web page or in a web worker';
    }
    var PACKAGE_NAME = './2zsk.data';
    var REMOTE_PACKAGE_BASE = '2zsk.data';
    if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
      Module['locateFile'] = Module['locateFilePackage'];
      err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
    }
    var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
  
    var REMOTE_PACKAGE_SIZE = metadata.remote_package_size;
    var PACKAGE_UUID = metadata.package_uuid;
  
    function fetchRemotePackage(packageName, packageSize, callback, errback) {
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
          if (Module['setStatus']) Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
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

    function DataRequest(start, end, audio) {
      this.start = start;
      this.end = end;
      this.audio = audio;
    }
    DataRequest.prototype = {
      requests: {},
      open: function(mode, name) {
        this.name = name;
        this.requests[name] = this;
        Module['addRunDependency']('fp ' + this.name);
      },
      send: function() {},
      onload: function() {
        var byteArray = this.byteArray.subarray(this.start, this.end);
        this.finish(byteArray);
      },
      finish: function(byteArray) {
        var that = this;

        Module['FS_createDataFile'](this.name, null, byteArray, true, true, true); // canOwn this data in the filesystem, it is a slide into the heap that will never change
        Module['removeRunDependency']('fp ' + that.name);

        this.requests[this.name] = null;
      }
    };

  
    function processPackageData(arrayBuffer) {
      Module.finishedDataFileDownloads++;
      assert(arrayBuffer, 'Loading data file failed.');
      assert(arrayBuffer instanceof ArrayBuffer, 'bad input to processPackageData');
      var byteArray = new Uint8Array(arrayBuffer);
      var curr;
      
          var compressedData = {"data":null,"cachedOffset":152158,"cachedIndexes":[-1,-1],"cachedChunks":[null,null],"offsets":[0,1051,2166,3234,4358,5433,6504,7580,8669,9743,10845,11918,13025,14103,15165,16241,17313,18400,19459,20526,21622,22712,23771,24866,25961,27079,28176,29284,30372,31498,32573,33635,34705,35790,36902,37982,39054,40126,41200,42322,43413,44514,45603,46704,47774,48876,49985,51095,52210,53324,54411,55474,56530,57586,58687,59750,60817,61894,62988,64105,65224,66306,67426,68516,69601,70665,71737,72826,73914,75001,76106,77219,78311,79384,80472,81531,82641,83688,84751,85834,86909,87980,89019,90087,91177,92272,93364,94474,95493,96590,97664,98739,99842,100939,102071,103133,104203,105279,106350,107469,108561,109639,110719,111789,112883,113994,115058,116135,117206,118299,119406,120493,121560,122620,123698,124780,125882,126990,128073,129157,130202,131286,132347,133425,134535,135620,136683,137749,138819,139943,141053,142169,143243,144330,145380,146485,147555,148633,149690,150764,151832],"sizes":[1051,1115,1068,1124,1075,1071,1076,1089,1074,1102,1073,1107,1078,1062,1076,1072,1087,1059,1067,1096,1090,1059,1095,1095,1118,1097,1108,1088,1126,1075,1062,1070,1085,1112,1080,1072,1072,1074,1122,1091,1101,1089,1101,1070,1102,1109,1110,1115,1114,1087,1063,1056,1056,1101,1063,1067,1077,1094,1117,1119,1082,1120,1090,1085,1064,1072,1089,1088,1087,1105,1113,1092,1073,1088,1059,1110,1047,1063,1083,1075,1071,1039,1068,1090,1095,1092,1110,1019,1097,1074,1075,1103,1097,1132,1062,1070,1076,1071,1119,1092,1078,1080,1070,1094,1111,1064,1077,1071,1093,1107,1087,1067,1060,1078,1082,1102,1108,1083,1084,1045,1084,1061,1078,1110,1085,1063,1066,1070,1124,1110,1116,1074,1087,1050,1105,1070,1078,1057,1074,1068,326],"successes":[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]}
;
          compressedData.data = byteArray;
          assert(typeof LZ4 === 'object', 'LZ4 not present - was your app build with  -s LZ4=1  ?');
          LZ4.loadPackage({ 'metadata': metadata, 'compressedData': compressedData });
          Module['removeRunDependency']('datafile_./2zsk.data');
    
    };
    Module['addRunDependency']('datafile_./2zsk.data');
  
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
 loadPackage({"files": [{"start": 0, "audio": 0, "end": 287307, "filename": "/2zsk.pdb"}], "remote_package_size": 156254, "package_uuid": "37bfb1c1-4010-402d-808b-c6e3229a46bc"});

})();
