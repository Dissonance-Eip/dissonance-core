// Test script for Hann and Hamming window functions

const core = require('./build/Release/dissonance_core.node');

console.log('Testing Window Functions\n' + '='.repeat(50));

// Test 1: Generate Hann window
console.log('\n1. Generate Hann Window (size=8)');
try {
    const hannWindow = core.generateWindow('hann', 8);
    console.log('   Hann window:', hannWindow.map(v => v.toFixed(4)).join(', '));
    
    // Verify it starts and ends at 0 (or very close to 0)
    if (Math.abs(hannWindow[0]) < 0.001 && Math.abs(hannWindow[7]) < 0.001) {
        console.log('   ✓ Correctly tapers to ~0 at edges');
    }
} catch (e) {
    console.error('   ✗ Error:', e.message);
}

// Test 2: Generate Hamming window
console.log('\n2. Generate Hamming Window (size=8)');
try {
    const hammingWindow = core.generateWindow('hamming', 8);
    console.log('   Hamming window:', hammingWindow.map(v => v.toFixed(4)).join(', '));
    
    // Verify it doesn't reach 0 at the edges (should be ~0.08)
    if (hammingWindow[0] > 0.05 && hammingWindow[7] > 0.05) {
        console.log('   ✓ Correctly stays above 0 at edges (~0.08)');
    }
} catch (e) {
    console.error('   ✗ Error:', e.message);
}

// Test 3: Compare larger window sizes
console.log('\n3. Generate larger windows (size=512)');
try {
    const hann512 = core.generateWindow('hann', 512);
    const hamming512 = core.generateWindow('hamming', 512);
    
    console.log('   Hann[0]:', hann512[0].toFixed(6), 'Hann[256]:', hann512[256].toFixed(6));
    console.log('   Hamming[0]:', hamming512[0].toFixed(6), 'Hamming[256]:', hamming512[256].toFixed(6));
    
    // Both should peak at 1.0 in the middle
    if (Math.abs(hann512[256] - 1.0) < 0.01 && Math.abs(hamming512[256] - 1.0) < 0.01) {
        console.log('   ✓ Both windows peak at ~1.0 in the center');
    }
} catch (e) {
    console.error('   ✗ Error:', e.message);
}

// Test 4: Apply window to sample data
console.log('\n4. Apply Window to Sample Data');
try {
    const samples = [1000, 2000, 3000, 4000, 3000, 2000, 1000, 500];
    const window = core.generateWindow('hann', 8);
    const windowed = core.applyWindow(samples, window);
    
    console.log('   Original:', samples.join(', '));
    console.log('   Window:', window.map(v => v.toFixed(3)).join(', '));
    console.log('   Windowed:', windowed.map(v => v.toFixed(1)).join(', '));
    
    // Check that edges are attenuated
    if (windowed[0] < samples[0] && windowed[7] < samples[7]) {
        console.log('   ✓ Edges are correctly attenuated');
    }
} catch (e) {
    console.error('   ✗ Error:', e.message);
}

// Test 5: Error handling - invalid window type
console.log('\n5. Error Handling - Invalid Window Type');
try {
    core.generateWindow('blackman', 8);
    console.error('   ✗ Should have thrown error for invalid type');
} catch (e) {
    console.log('   ✓ Correctly throws error:', e.message);
}

// Test 6: Error handling - mismatched sizes
console.log('\n6. Error Handling - Mismatched Array Sizes');
try {
    const samples = [1, 2, 3, 4];
    const window = [0.5, 0.5, 0.5];
    core.applyWindow(samples, window);
    console.error('   ✗ Should have thrown error for size mismatch');
} catch (e) {
    console.log('   ✓ Correctly throws error:', e.message);
}

console.log('\n' + '='.repeat(50));
console.log('Window functions test complete!\n');
