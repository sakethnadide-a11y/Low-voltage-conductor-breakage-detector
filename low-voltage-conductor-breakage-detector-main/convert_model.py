    import os

    # Path to your TensorFlow Lite model file
    tflite_model_path = 'anomaly_detector.tflite'

    # Path for the output header file
    header_file_path = 'model_data.h'

    # Read the TFLite model file in binary format
    with open(tflite_model_path, 'rb') as f:
        tflite_model_bytes = f.read()

    # Convert the bytes to a C++ array format
    c_array_string = ', '.join([f'0x{byte:02x}' for byte in tflite_model_bytes])

    # Write the C++ header file
    with open(header_file_path, 'w') as f:
        f.write('#ifndef MODEL_DATA_H\n')
        f.write('#define MODEL_DATA_H\n\n')
        f.write('alignas(8) const unsigned char g_model_data[] = {\n')
        f.write(f'  {c_array_string}\n')
        f.write('};\n')
        f.write(f'const int g_model_data_len = {len(tflite_model_bytes)};\n\n')
        f.write('#endif // MODEL_DATA_H\n')

    print(f"Successfully converted '{tflite_model_path}' to '{header_file_path}'.")