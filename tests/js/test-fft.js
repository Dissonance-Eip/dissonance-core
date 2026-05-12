// Simple FFT/iFFT test script
const core = require('../../build/Release/dissonance_core.node');

function approxEqual(a, b, eps = 1e-5) {
  return Math.abs(a - b) < eps;
}

console.log('Testing FFT/iFFT minimal chain');
console.log('='.repeat(50));

// 1) FFT of an impulse should be flat: all bins have real=1, imag=0
const impulse = new Float32Array([1, 0, 0, 0]);
const spectrum = core.fft(impulse);
const allOnesReal = Array.from(spectrum.real).every(v => approxEqual(v, 1));
const allZerosImag = Array.from(spectrum.imag).every(v => approxEqual(v, 0));
console.log('\nFFT impulse -> flat real:', allOnesReal ? 'PASS' : 'FAIL');
console.log('FFT impulse -> zero imag:', allZerosImag ? 'PASS' : 'FAIL');

// 2) iFFT of that spectrum should reconstruct the impulse
const reconstructedImpulse = core.ifft(spectrum);
const impulseMatches = Array.from(reconstructedImpulse).every((v, i) => approxEqual(v, impulse[i]));
console.log('iFFT reconstructs impulse:', impulseMatches ? 'PASS' : 'FAIL');

// 3) Round-trip on a small real signal
const signal = new Float32Array([0, 1, 0, -1]);
const signalSpectrum = core.fft(signal);
const signalBack = core.ifft(signalSpectrum);
const roundTripOk = Array.from(signalBack).every((v, i) => approxEqual(v, signal[i]));
console.log('\nRound-trip signal -> spectrum -> signal:', roundTripOk ? 'PASS' : 'FAIL');

// 4) Magnitude sanity check — length matches bin count
const magnitudes = core.fftMagnitude(signalSpectrum);
const lenOk = magnitudes.length === signal.length;
console.log('Magnitude length correct:', lenOk ? 'PASS' : 'FAIL');

// 5) Spectrum shape check — real and imag are Float32Arrays of same length
const shapeOk =
  spectrum.real instanceof Float32Array &&
  spectrum.imag instanceof Float32Array &&
  spectrum.real.length === impulse.length &&
  spectrum.imag.length === impulse.length;
console.log('Spectrum has {real, imag} Float32Arrays:', shapeOk ? 'PASS' : 'FAIL');

console.log('\nDone.');
