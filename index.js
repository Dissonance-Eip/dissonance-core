const addon = require("./build/Release/dissonance_core.node");

if (typeof addon.process !== "function")
  throw new Error("dissonance-core: missing export: process");
if (typeof addon.readMetadata !== "function")
  throw new Error("dissonance-core: missing export: readMetadata");
if (typeof addon.writeTags !== "function")
  throw new Error("dissonance-core: missing export: writeTags");

module.exports = addon;
