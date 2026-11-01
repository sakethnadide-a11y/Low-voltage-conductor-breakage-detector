import pandas as pd

# This reads your CSV file into a Pandas DataFrame, like a single-column spreadsheet.
df = pd.read_csv('current_reading.csv')

# To confirm it loaded correctly, you can print the first few rows.
print("Raw Data:")
print(df.head())


from sklearn.preprocessing import MinMaxScaler

# The 'MinMaxScaler' tool learns the smallest and largest values in your data.
scaler = MinMaxScaler()

# 'fit_transform' learns the scaling rule and applies it to your data.
# The result is 'scaled_data', where all your original current values are now between 0 and 1.
scaled_data = scaler.fit_transform(df)

print("\nScaled Data:")
print(scaled_data[:5]) # Print the first 5 scaled values



import tensorflow as tf
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Dense, Input

# The number of features (data columns) in your dataset.
# Since you only have 'current', the dimension is 1.
input_dim = 1

# Define the Autoencoder Model
# The model will have an "encoder" part to compress the data
# and a "decoder" part to decompress it.
input_layer = Input(shape=(input_dim,)) # Input layer receives the data
encoder_layer = Dense(8, activation="relu")(input_layer)
bottleneck = Dense(4, activation="relu")(encoder_layer) # This is the compressed representation
decoder_layer = Dense(8, activation="relu")(bottleneck)
output_layer = Dense(input_dim, activation="sigmoid")(decoder_layer) # Output should match the input shape

# Create the full model by connecting the input and output layers
autoencoder = Model(inputs=input_layer, outputs=output_layer)

# Compile the model
# The 'adam' optimizer adjusts the model's weights during training.
# The 'mae' (Mean Absolute Error) is the metric the model uses to measure its performance.
autoencoder.compile(optimizer='adam', loss='mae')

# Train the model
# 'scaled_data' is both the input and the target output, because the goal is for the model to reproduce its own input.
# 'epochs' is the number of times the model will go through the entire dataset.
# 'validation_data' is used to monitor training progress. Since we only have "normal" data, we use the same data for validation.
autoencoder.fit(scaled_data, scaled_data,
                epochs=50,
                batch_size=32,
                shuffle=True,
                validation_data=(scaled_data, scaled_data))

print("Model training is complete!")


import numpy as np

# Get the model's predictions on the training data
predictions = autoencoder.predict(scaled_data)

# Calculate the reconstruction error for each data point
reconstruction_error = np.mean(np.abs(predictions - scaled_data), axis=1)

# Find the threshold (e.g., 95th percentile of the errors)
threshold = np.percentile(reconstruction_error, 95)
print(f"Calculated anomaly threshold: {threshold}")


# After the training and threshold calculation code...
converter = tf.lite.TFLiteConverter.from_keras_model(autoencoder)
tflite_model = converter.convert()

# Save the converted model to a file
with open('anomaly_detector.tflite', 'wb') as f:
    f.write(tflite_model)

print("Model saved as anomaly_detector.tflite")


import numpy as np

# Get model predictions (reconstructions)
predictions = autoencoder.predict(scaled_data)

# Calculate mean absolute error for each sample
reconstruction_error = np.mean(np.abs(predictions - scaled_data), axis=1)

# Calculate average reconstruction error over all samples
average_mae = np.mean(reconstruction_error)

print(f"Average Reconstruction Error (MAE): {average_mae}")



