const addon = require("./build/Release/dissonance_core.node");

const REQUIRED = [
  "process",
  "inspect",
  "fft",
  "ifft",
  "fftMagnitude",
  "generateWindow",
  "applyWindow",
];

const missing = REQUIRED.filter((k) => typeof addon[k] !== "function");
if (missing.length) {
  throw new Error(
    `dissonance-core: missing exports: ${missing.join(", ")}`
  );
}

module.exports = addon;
