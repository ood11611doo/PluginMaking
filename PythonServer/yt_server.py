from flask import Flask, request, jsonify
import yt_dlp
import sys

app = Flask(__name__)

@app.route('/get_link', methods=['GET'])
def get_link():
    # 1. Get URL from Unreal
    video_url = request.args.get('url')
    if not video_url:
        print("Error: Received request with no URL")
        return jsonify({"status": "error", "message": "No URL provided"}), 400

    print(f"Web-Job: Extracting link for -> {video_url}")

    # 2. Configure yt-dlp for pure extraction
    ydl_opts = {
        'format': 'best[ext=mp4]', # Ensure VLC-friendly MP4
        'quiet': True,
        'no_warnings': True,
    }

    try:
        with yt_dlp.YoutubeDL(ydl_opts) as ydl:
            # This is the "Web Job" part
            info = ydl.extract_info(video_url, download=False)
            
            # 3. Return everything Unreal needs to know
            return jsonify({
                "status": "success",
                "title": info.get('title'),
                "direct_url": info.get('url'),
                "duration": info.get('duration'),
                "thumbnail": info.get('thumbnail')
            })
    except Exception as e:
        print(f"Extraction Failed: {str(e)}")
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    print("------------------------------------------")
    print(" PORTABLE WEB-JOB SERVER IS ONLINE")
    print(" URL: http://127.0.0.1:5000/get_link")
    print("------------------------------------------")
    # Port 5000 is standard for local web-jobs
    app.run(host='0.0.0.0', port=5000, debug=False)