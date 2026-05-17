const addon = require("./build/Release/dissonance_core.node");

if (typeof addon.process !== "function")
  throw new Error("dissonance-core: missing export: process");

module.exports = addon;
