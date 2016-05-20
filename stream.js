var util = require('util');
var binding = require('./incremental');
var algorithms = {};

exports.getHashes = function() {
  return Object.keys(binding)
        .filter(function(name) {
          return 'function' === typeof binding[name];
        })
        .map(function(name) { return name.toLowerCase(); });
};

Object.keys(binding).forEach(function(name) {
  if (binding.hasOwnProperty(name) && 'function' === typeof binding[name]) {
    algorithms[name.toLowerCase()] = binding[name];
  }
});

algorithms.murmurhash3a = algorithms.murmurhash32 = algorithms.murmurhash32x86 = algorithms.murmurhash;

/* from nodejs lib/crypto.js */

var LazyTransform = require('./lazy_transform');

/**
 * Creates and returns a MurmurHash object that can be used to generate murmurhash digests.
 * 
 * Except murmur's `seed` option, the rest of the options are passed to
 * stream.Transform constructor.
 *
 * @param {string|MurmurHash} algorithm|hasher - one of available algorithms
 *                            or a murmur hasher instance
 * @param {number|object} seed|options - hasher options
 **/
exports.createHash = exports.MurmurHash = MurmurHash;
function MurmurHash(algorithm, options) {
  var seed;

  if (!(this instanceof MurmurHash))
    return new MurmurHash(algorithm, options);

  if (options && 'object' === typeof options) {
    seed = options.seed;
  } else {
    seed = options; options = undefined;
  }

  if (algorithm instanceof MurmurHash) {
    this._handle = new algorithm._handle.constructor(algorithm._handle);
  } else if (algorithm) {
    // handle object from json
    if ('object' === typeof algorithm) {
      seed = algorithm.seed;
      algorithm = algorithm.type;
    }
    var Handle = algorithms[algorithm.toLowerCase()];
    if (Handle) {
      this._handle = new Handle(seed);
    } else {
      throw new Error("Algorithm not supported");
    }
  } else {
    throw new TypeError("Must give algorithm string or MurmurHash instance");
  }

  LazyTransform.call(this, options);
}

util.inherits(MurmurHash, LazyTransform);

MurmurHash.prototype._transform = function(chunk, encoding, callback) {
  this._handle.update(chunk, encoding);
  callback();
};

MurmurHash.prototype._flush = function(callback) {
  this.push(this._handle.digest());
  callback();
};

MurmurHash.prototype.update = function(data, encoding) {
  this._handle.update(data, encoding);
  return this;
};

MurmurHash.prototype.digest = function(outputEncoding) {
  return this._handle.digest(outputEncoding);
};

MurmurHash.prototype.serialize = function(type, offset) {
  return this._handle.serialize(type, offset);
};

MurmurHash.prototype.copy = function(target) {
  this._handle.copy(target && target._handle);
  return target;
};

MurmurHash.prototype.toJSON = function() {
  var handle = this._handle;
  return {
    type: handle.constructor.name,
    seed: handle.toJSON()
  };
};

Object.defineProperty(MurmurHash.prototype, 'total', {
  get: function() {
    return this._handle.total;
  },
  enumerable: true,
  configurable: true
});

Object.defineProperty(MurmurHash.prototype, 'SERIAL_BYTE_LENGTH', {
  get: function() {
    Object.defineProperty(this, 'SERIAL_BYTE_LENGTH', {
      enumerable: true,
      writable: true,
      configurable: true,
      value: this._handle.SERIAL_BYTE_LENGTH
    });
    return this.SERIAL_BYTE_LENGTH;
  },
  enumerable: true,
  configurable: true,
});