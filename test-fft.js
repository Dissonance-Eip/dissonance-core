// Simple FFT/iFFT test script
const core = require('./build/Release/dissonance_core.node');

function approxEqual(a, b, eps = 1e-6) {
  return Math.abs(a - b) < eps;
}

console.log('Testing FFT/iFFT minimal chain');
console.log('='.repeat(50));

// 1) FFT of an impulse should be flat
const impulse = [1, 0, 0, 0];
const spectrum = core.fft(impulse);
const allOnes = spectrum.every(bin => approxEqual(bin.real, 1) && approxEqual(bin.imag, 0));
console.log('\nFFT impulse -> flat spectrum:', allOnes ? 'PASS' : 'FAIL');

// 2) iFFT of that spectrum should reconstruct the impulse
const reconstructedImpulse = core.ifft(spectrum);
const impulseMatches = reconstructedImpulse.every((v, i) => approxEqual(v, impulse[i]));
console.log('iFFT reconstructs impulse:', impulseMatches ? 'PASS' : 'FAIL');

// 3) Round-trip on a small real signal
const signal = [0, 1, 0, -1];
const signalSpectrum = core.fft(signal);
const signalBack = core.ifft(signalSpectrum);
const roundTripOk = signalBack.every((v, i) => approxEqual(v, signal[i]));
console.log('\nRound-trip signal -> spectrum -> signal:', roundTripOk ? 'PASS' : 'FAIL');

// 4) Magnitude sanity check
const magnitudes = core.fftMagnitude(signalSpectrum);
console.log('Magnitude length correct:', magnitudes.length === signalSpectrum.length ? 'PASS' : 'FAIL');

console.log('\nDone.');
