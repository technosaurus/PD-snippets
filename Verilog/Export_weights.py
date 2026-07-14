#!/usr/bin/env python3
"""
==============================================================================
WEIGHT COMPILER & STREAM GENERATOR FOR 128-CORE 3D WAVEFRONT FABRIC
Generates the serialized boot-stream required to provision the network matrix.
==============================================================================
"""

import numpy as np

def generate_mock_trained_network():
    """Simulates a pre-trained neural network layer model.
    Layer 0: 64 cores, each with 64 weights.
    Layer 1: 64 cores, each with 64 weights.
    """
    # Create quantized INT8 array structures ranging from 0 to 255
    layer_0_weights = np.random.randint(0, 256, size=(64, 64), dtype=np.uint8)
    layer_1_weights = np.random.randint(0, 256, size=(64, 64), dtype=np.uint8)
    return layer_0_weights, layer_1_weights

def serialize_wavefront_stream(l0_data, l1_data):
    """Flattens the network data into the precise cascading boot sequence.
    Because data cascades through Layer 1 down into Layer 0, the stream 
    must feed the memory buffers starting from the absolute deepest cells first.
    """
    serialized_stream = []
    
    # Pack Layer 1 parameters first (Deep layer)
    for core_idx in range(64):
        for weight_idx in range(64):
            serialized_stream.append(l1_data[core_idx, weight_idx])
            
    # Pack Layer 0 parameters next (Entry layer)
    for core_idx in range(64):
        for weight_idx in range(64):
            serialized_stream.append(l0_data[core_idx, weight_idx])
            
    return serialized_stream

def save_hardware_files(stream_data, filename="boot_weights.hex"):
    """Writes out a hex verification file formatted directly for FPGA 
    testbenches and hardware flashing routines.
    """
    with open(filename, "w") as f:
        for byte in stream_data:
            # Write out pure hex characters to match Verilog $readmemh formats
            f.write(f"{byte:02X}\n")
    print(f"[SUCCESS] Wavefront boot-stream compiled: {len(stream_data)} bytes written to {filename}")

if __name__ == "__main__":
    print("[INIT] Launching Wavefront Array Compiling System...")
    
    # 1. Harvest weight matrices
    l0, l1 = generate_mock_trained_network()
    
    # 2. Serialize into the cascading streaming order
    boot_stream = serialize_wavefront_stream(l0, l1)
    
    # 3. Export verification binary images
    save_hardware_files(boot_stream)
