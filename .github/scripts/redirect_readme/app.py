from flask import Flask, redirect, Response
import requests
import re

app = Flask(__name__)

# Google Cloud Storage base URL
GCS_BASE_URL = "https://storage.googleapis.com/implot3d"

# Route to handle discussion redirects
@app.route('/discussion_<int:discussion_id>')
def redirect_to_discussion(discussion_id):
    # Construct the URL for the SVG file in GCS
    svg_url = f"{GCS_BASE_URL}/discussion_{discussion_id}.svg"

    # Fetch the SVG content from GCS
    response = requests.get(svg_url)

    if response.status_code == 200:
        svg_content = response.text

        # Extract the GitHub discussion URL from SVG comments
        match = re.search(r'<!--\s*(https://github.com/brenocq/implot3d/discussions/\d+)\s*-->', svg_content)

        if match:
            discussion_url = match.group(1)
            # Redirect to the extracted URL
            return redirect(discussion_url, code=302)
        else:
            # Return 404 if no URL is found in the SVG
            return Response("Discussion link not found in SVG.", status=404)
    else:
        # Return 404 if the SVG does not exist
        return Response("SVG not found.", status=404)

# Default route
@app.route('/')
def home():
    return "Hello from implot3d! This is the redirect service."
