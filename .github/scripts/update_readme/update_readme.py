import os
import requests
import base64
import html
from datetime import datetime
from collections import Counter
from google.cloud import storage

# GCloud
storage_client = storage.Client()
bucket = storage_client.get_bucket('implot3d')

# GitHub token
GITHUB_TOKEN = os.environ['GITHUB_TOKEN']
if not GITHUB_TOKEN:
    raise ValueError("GITHUB_TOKEN environment variable is not set")

def generate_status_svg(label_text, label_color, count):
    width = 140
    height = 120
    label_width = len(label_text) * 8 + 5

    # Create SVG content
    svg = f"""
    <svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">
        <defs>
            <filter id="shadow" x="-10%" y="-10%" width="120%" height="120%">
                <feDropShadow dx="2" dy="2" stdDeviation="2" flood-color="black"/>
            </filter>
        </defs>

        <!-- Card Background with Shadow -->
        <rect x="5" y="5" width="{width-10}" height="{height-10}" rx="12" fill="#212830" filter="url(#shadow)"/>

        <!-- Label -->
        <g transform="translate({width/2}, 20)">
            <rect x="{-label_width/2}" y="0" width="{label_width}" height="24" rx="12" fill="{label_color}" fill-opacity="0.2" stroke="{label_color}" stroke-width="0.5"/>
            <text x="0" y="17" font-size="14" fill="{label_color}" font-family="Arial" text-anchor="middle">{label_text}</text>
        </g>

        <!-- Text -->
        <text x="{width/2}" y="90" font-size="40" fill="#9198a1" font-family="Arial" text-anchor="middle">{count}</text>
    </svg>
    """

    return svg

def fetch_base64_image(url):
    """
    Fetch an image from the given URL and return its Base64-encoded representation.
    """
    response = requests.get(url)
    if response.status_code == 200:
        return base64.b64encode(response.content).decode('utf-8')
    else:
        raise ValueError(f"Failed to fetch image from {url}, status code: {response.status_code}")

def generate_discussion_svg(title, emoji, labels, category, upvotes, comments, author, created_at, last_comment_by, last_comment_at, discussion_url, participants):
    width = 820
    height = 120

    # Emoji
    emoji_size = 20 # 16 font size equal 20x19 px
    emoji_box_x = 30
    emoji_box_size = 54

    # Upvotes
    upvote_width = len(str(upvotes)) * 10 + 30
    upvote_x_center = 768
    upvote_rect_x = upvote_x_center - upvote_width / 2

    # Comments
    comment_width = len(str(comments)) * 10 + 20
    comment_x_center = 768
    comment_rect_x = comment_x_center - comment_width / 2

    # Profile pictures
    profile_size = 40
    profile_gap = 20
    profile_x = 710
    profile_y = height / 2

    # Format dates
    created_at_formatted = datetime.strptime(created_at, "%Y-%m-%dT%H:%M:%SZ").strftime("%d %b %Y")
    last_comment_at_formatted = (
        datetime.strptime(last_comment_at, "%Y-%m-%dT%H:%M:%SZ").strftime("%d %b %Y") if last_comment_at else None
    )

    # Build the contributor text
    contributor_comment =  f'<tspan style="text-decoration: underline;">{author}</tspan> started on {created_at_formatted}.'
    if last_comment_by and last_comment_at_formatted:
        contributor_comment = contributor_comment + f' Last comment by <tspan style="text-decoration: underline;">{last_comment_by}</tspan> on {last_comment_at_formatted}.'

    # Create SVG content
    svg = f"""
    <svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">
        <!-- {discussion_url} -->
        <defs>
            <filter id="shadow" x="-10%" y="-10%" width="120%" height="120%">
                <feDropShadow dx="2" dy="2" stdDeviation="2" flood-color="black"/>
            </filter>
            <clipPath id="circle-clip">
                <circle cx="0" cy="0" r="{profile_size / 2}" />
            </clipPath>
        </defs>

        <!-- Card Background with Shadow -->
        <rect x="5" y="5" width="{width-10}" height="{height-10}" rx="12" fill="#212830" filter="url(#shadow)"/>

        <!-- Emoji Icon -->
        <rect x="{emoji_box_x}" y="{(height - emoji_box_size)/2}" width="{emoji_box_size}" height="{emoji_box_size}" rx="6" fill="#57606a"/>
        <text x="{emoji_box_x + emoji_box_size/2 - emoji_size/2}" y="{(height + emoji_size)/2-4}" font-size="16">{emoji}</text>

        <!-- Title -->
        <text x="100" y="40" font-size="20" fill="#9198a1" font-family="Arial" font-weight="bold">{html.escape(title)}</text>

        <!-- Labels -->
        <g transform="translate(100, 50)">
    """
    x_offset = 0
    for label in labels:
        label_width = len(label['text']) * 8 + 5
        svg += f"""
        <rect x="{x_offset}" y="0" width="{label_width}" height="24" rx="12" fill="{label['color']}" fill-opacity="0.2" stroke="{label['color']}" stroke-width="0.5"/>
        <text x="{x_offset + label_width / 2}" y="17" font-size="14" fill="{label['color']}" font-family="Arial" text-anchor="middle">{label['text']}</text>
        """
        x_offset += label_width + 10

    svg += f"""
        </g>

        <!-- Username -->
        <text x="100" y="95" font-size="14" fill="#9198a1" font-family="Arial">{contributor_comment}</text>

        <!-- Upvote Button -->
        <g transform="translate({upvote_rect_x}, 45)">
            <rect x="0" y="-12" width="{upvote_width}" height="24" rx="12" fill="#478be6" fill-opacity="0.2" stroke="#478be6" stroke-width="0.5"/>
            <path d="M3.47 7.78a.75.75 0 0 1 0-1.06l4.25-4.25a.75.75 0 0 1 1.06 0l4.25 4.25a.751.751 0 0 1-.018 1.042.751.751 0 0 1-1.042.018L9 4.81v7.44a.75.75 0 0 1-1.5 0V4.81L4.53 7.78a.75.75 0 0 1-1.06 0Z" fill="#478be6" transform="translate(5, -8) scale(1.14, 1.14)"/>
            <text x="{upvote_width / 2 + 8}" y="6" font-size="16" fill="#478be6" font-family="Arial" text-anchor="middle">{upvotes}</text>
        </g>

        <!-- Comments -->
        <g transform="translate({comment_rect_x}, 75)">
            <path d="M1 2.75C1 1.784 1.784 1 2.75 1h10.5c.966 0 1.75.784 1.75 1.75v7.5A1.75 1.75 0 0 1 13.25 12H9.06l-2.573 2.573A1.458 1.458 0 0 1 4 13.543V12H2.75A1.75 1.75 0 0 1 1 10.25Zm1.75-.25a.25.25 0 0 0-.25.25v7.5c0 .138.112.25.25.25h2a.75.75 0 0 1 .75.75v2.19l2.72-2.72a.749.749 0 0 1 .53-.22h4.5a.25.25 0 0 0 .25-.25v-7.5a.25.25 0 0 0-.25-.25Z" fill="#8b949e" transform="scale(1.14, 1.14)"/>
            <text x="20" y="14" font-size="16" fill="#8b949e" font-family="Arial">{comments}</text>
        </g>
    """

    # Render participant avatars
    svg += """
        <!-- Avatars -->
        """
    for participant in participants[:10]:
        try:
            base64_image = fetch_base64_image(participant)
            svg += f"""
            <g transform="translate({profile_x}, {profile_y})">
                <image x="{-profile_size / 2}" y="{-profile_size / 2}" width="{profile_size}" height="{profile_size}" href="data:image/png;base64,{base64_image}" clip-path="url(#circle-clip)" />
            </g>
            """
            profile_x -= profile_gap
        except ValueError as e:
            print(f"Failed to fetch image for participant: {participant}, error: {e}")
    svg += """
    </svg>
        """

    return svg

def update_svgs():
    url = "https://api.github.com/graphql"

    headers = {
        "Authorization": f"Bearer {GITHUB_TOKEN}",
        "Content-Type": "application/json"
    }

    query = """
    {
      repository(owner: "brenocq", name: "implot3d") {
        discussions(first: 100, categoryId: "DIC_kwDONQXA0M4ClSCg", orderBy: {field: UPDATED_AT, direction: DESC}) {
          nodes {
            title
            url
            createdAt
            updatedAt
            upvoteCount
            comments(first: 20) {
              totalCount
              nodes {
                author {
                  login
                  avatarUrl(size: 40)
                }
                createdAt
                replies(first: 20) {
                  totalCount
                  nodes {
                    author {
                      login
                      avatarUrl(size: 40)
                    }
                    createdAt
                  }
                }
              }
            }
            labels(first: 5) {
              nodes {
                name
                color
              }
            }
            category {
              name
            }
            author {
              login
              avatarUrl(size: 40)
            }
          }
        }
      }
    }
    """

    response = requests.post(url, headers=headers, json={"query": query})
    data = response.json()

    if response.status_code == 200 and 'data' in data:
        discussions = data["data"]["repository"]["discussions"]["nodes"]

        ############### Generate status SVGs ###############
        # Count number of discussions by status
        status_counter = Counter({
            'status:idea': 0,
            'status:todo': 0,
            'status:doing': 0,
            'status:review': 0,
            'status:done': 0
        })
        for discussion in discussions:
            labels = [label['name'] for label in discussion['labels']['nodes']]
            for status in status_counter.keys():
                if status in labels:
                    status_counter[status] += 1
        # Generate SVGs for Each Status
        status_colors = {
            'status:idea': '#5DADE2',
            'status:todo': '#3498DB',
            'status:doing': '#F1C40F',
            'status:review': '#E67E22',
            'status:done': '#27AE60'
        }

        for status, count in status_counter.items():
            print(f"Generating SVG for: {status}")

            svg_status_output = generate_status_svg(
                label_text=status,
                label_color=status_colors[status],
                count=count
            )

            # Save and Upload SVG
            filename = f"{status.split(':')[1]}.svg"
            with open(filename, "w") as f:
                f.write(svg_status_output)
            print(f"Saved SVG as {filename}")

            # Upload SVG to GCloud
            blob = bucket.blob(filename)
            blob.upload_from_filename(filename)
            print(f"Uploaded {filename} to google storage")
            print("-" * 60)

        ############### Generate discussion SVGs ###############
        # Generate SVGs for 5 most recent discussions
        for i, discussion in enumerate(discussions):
            if i >= 5:
                break
            print(f"Generating SVG for: {discussion['title']}")

            # List all participants
            participants = {}
            participants[discussion['author']['login']] = discussion['author']['avatarUrl']
            for comment in discussion['comments']['nodes']:
                participants[comment['author']['login']] = comment['author']['avatarUrl']
                for reply in comment['replies']['nodes']:
                    participants[reply['author']['login']] = reply['author']['avatarUrl']

            # Calculate total comments (including replies)
            total_comments = 0
            last_comment_by = None
            last_comment_at = None
            for comment in discussion['comments']['nodes']:
                total_comments += 1  # Top-level comment
                total_comments += comment['replies']['totalCount']  # Add replies

                # Track the last comment
                if last_comment_at is None or comment['createdAt'] > last_comment_at:
                    last_comment_by = comment['author']['login']
                    last_comment_at = comment['createdAt']

                # Track the last comment in case it is a reply
                for reply in comment['replies']['nodes']:
                    if reply['createdAt'] > last_comment_at:
                        last_comment_by = reply['author']['login']
                        last_comment_at = reply['createdAt']

            # Extract labels
            labels = [
                {"text": label['name'], "color": f"#{label['color']}"}
                for label in discussion['labels']['nodes']
            ]

            # Generate the SVG for each discussion
            svg_output = generate_discussion_svg(
                title=discussion['title'],
                emoji="💡",
                labels=labels,
                category=discussion['category']['name'],
                upvotes=discussion['upvoteCount'],
                comments=total_comments,
                author=discussion['author']['login'],
                created_at=discussion['createdAt'],
                last_comment_by=last_comment_by,
                last_comment_at=last_comment_at,
                discussion_url=discussion['url'],
                participants=list(participants.values())
            )

            # Save each SVG to a unique file
            filename = f"discussion_{i}.svg"
            with open(filename, "w") as f:
                f.write(svg_output)

            print(f"Saved SVG as {filename}")

            # Upload SVG to GCloud
            blob = bucket.blob(filename)
            blob.upload_from_filename(filename)
            print(f"Uploaded {filename} to google storage")
            print("-" * 60)
    else:
        print("Error or No Data Returned")
        print(f"Response: {data}")

update_svgs()
