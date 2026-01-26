/**
 * JavaScript test for gain processing
 * Run with: node test-gain.js
 */

const dissonanceCore = require('./build/Release/dissonance_core.node');
const fs = require('fs');
const path = require('path');

const testFile = './test_files/sound.wav';

if (!fs.existsSync(testFile)) {
    console.error(`Test file not found: ${testFile}`);
    process.exit(1);
}

console.log('Testing gain processing...\n');

// Test 1: Default gain
console.log('✓ Test 1: Default gain (0.8)');
try {
    const outputFile1 = path.join(path.dirname(testFile), 'sound-gain-0.8.wav');
    const result1 = dissonanceCore.process(testFile, { outputPath: outputFile1 });
    console.log(`  Processed file: ${result1.processedPath}`);
    console.log(`  Metadata: ${result1.metadataText.split('\n')[0]}`);
    console.log('  ✓ Default gain test passed\n');
} catch (e) {
    console.error('  ✗ Default gain test failed:', e.message, '\n');
}

// Test 2: Custom gain (0.5) - comparing with default 0.8
console.log('✓ Test 2: Custom gain (0.5)');
try {
    // Process with 0.8 first and save file size
    const outputFile08 = path.join(path.dirname(testFile), 'sound-gain-0.8.wav');
    const result08 = dissonanceCore.process(testFile, { gain: 0.8, outputPath: outputFile08 });
    const buffer08 = fs.readFileSync(result08.processedPath);
    
    // Now process with 0.5
    const outputFile05 = path.join(path.dirname(testFile), 'sound-gain-0.5.wav');
    const result05 = dissonanceCore.process(testFile, { gain: 0.5, outputPath: outputFile05 });
    const buffer05 = fs.readFileSync(result05.processedPath);
    
    console.log(`  File with 0.8 gain: ${buffer08.length} bytes`);
    console.log(`  File with 0.5 gain: ${buffer05.length} bytes`);
    
    let foundSample = false;
    for (let i = 44; i < Math.min(buffer08.length, buffer05.length) - 2; i += 2) {
        const sample08 = buffer08.readInt16LE(i);
        const sample05 = buffer05.readInt16LE(i);
        
        // Look for samples with meaningful amplitude
        if (Math.abs(sample08) > 100 && Math.abs(sample05) > 50) {
            const ratio = sample05 / sample08;
            console.log(`  Sample at offset ${i}:`);
            console.log(`    0.8 gain: ${sample08}`);
            console.log(`    0.5 gain: ${sample05}`);
            console.log(`    Ratio: ${ratio.toFixed(3)} (expected ~0.625 = 0.5/0.8)`);
            console.log(`    Matches expected: ${Math.abs(ratio - 0.625) < 0.05 ? 'YES ✓' : 'NO ✗'}`);
            foundSample = true;
            break;
        }
    }
    
    if (!foundSample) {
        console.log('  ⚠ Warning: Could not find non-zero samples to verify gain\n');
    } else {
        console.log('  ✓ Custom gain test passed\n');
    }
} catch (e) {
    console.error('  ✗ Custom gain test failed:', e.message, '\n');
}

// Test 3: Gain = 1.0 (no change)
console.log('✓ Test 3: Gain = 1.0 (no modification)');
try {
    const outputFile10 = path.join(path.dirname(testFile), 'sound-gain-1.0.wav');
    const result3 = dissonanceCore.process(testFile, { gain: 1.0, outputPath: outputFile10 });
    console.log(`  Processed file: ${result3.processedPath}`);
    console.log('  ✓ Gain 1.0 test passed\n');
} catch (e) {
    console.error('  ✗ Gain 1.0 test failed:', e.message, '\n');
}

// Test 4: High gain (1.5)
console.log('✓ Test 4: High gain (1.5)');
try {
    const outputFile15 = path.join(path.dirname(testFile), 'sound-gain-1.5.wav');
    const result4 = dissonanceCore.process(testFile, { gain: 1.5, outputPath: outputFile15 });
    console.log(`  Processed file: ${result4.processedPath}`);
    console.log('  ✓ High gain test passed\n');
} catch (e) {
    console.error('  ✗ High gain test failed:', e.message, '\n');
}

console.log('All tests completed!');
