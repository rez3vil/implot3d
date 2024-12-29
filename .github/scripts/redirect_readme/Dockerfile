# Use an official lightweight Python image
FROM python:3.9-slim

# Set the working directory in the container
WORKDIR /app

# Copy the current directory contents into the container
COPY . .

# Install dependencies
RUN pip install -r requirements.txt

# Run Gunicorn to serve Flask
CMD ["gunicorn", "-w", "4", "-b", "0.0.0.0:8080", "app:app"]
