import os
import requests
import html
from datetime import datetime
from google.cloud import storage

# GCloud
storage_client = storage.Client()
bucket = storage_client.get_bucket('implot3d')

# GitHub token
GITHUB_TOKEN = os.environ['GITHUB_TOKEN']
if not GITHUB_TOKEN:
    raise ValueError("GITHUB_TOKEN environment variable is not set")

def generate_discussion_svg(title, emoji, labels, category, upvotes, comments, author, created_at, last_comment_by, last_comment_at, discussion_url):
    width = 820
    height = 120
    emoji_size = 20 # 16 font size equal 20x19 px
    emoji_box_x = 30
    emoji_box_size = 54
    upvote_width = len(str(upvotes)) * 10 + 30
    upvote_x_center = 760
    upvote_rect_x = upvote_x_center - upvote_width / 2

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
            <filter id="shadow" x="-20%" y="-20%" width="140%" height="140%">
                <feDropShadow dx="3" dy="3" stdDeviation="3" flood-color="black"/>
            </filter>
        </defs>

        <!-- Card Background with Shadow -->
        <rect x="10" y="10" width="800" height="100" rx="12" fill="#212830" filter="url(#shadow)"/>

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
        <g transform="translate(747, 75)">
            <path d="M1 2.75C1 1.784 1.784 1 2.75 1h10.5c.966 0 1.75.784 1.75 1.75v7.5A1.75 1.75 0 0 1 13.25 12H9.06l-2.573 2.573A1.458 1.458 0 0 1 4 13.543V12H2.75A1.75 1.75 0 0 1 1 10.25Zm1.75-.25a.25.25 0 0 0-.25.25v7.5c0 .138.112.25.25.25h2a.75.75 0 0 1 .75.75v2.19l2.72-2.72a.749.749 0 0 1 .53-.22h4.5a.25.25 0 0 0 .25-.25v-7.5a.25.25 0 0 0-.25-.25Z" fill="#8b949e" transform="scale(1.14, 1.14)"/>
            <text x="20" y="14" font-size="16" fill="#8b949e" font-family="Arial">{comments}</text>
        </g>
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
        discussions(first: 5, categoryId: "DIC_kwDONQXA0M4ClSCg", orderBy: {field: UPDATED_AT, direction: DESC}) {
          nodes {
            title
            url
            createdAt
            updatedAt
            upvoteCount
            comments(first: 10) {
              totalCount
              nodes {
                author {
                  login
                }
                createdAt
                replies(first: 10) {
                  totalCount
                  nodes {
                    author {
                      login
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

        for i, discussion in enumerate(discussions):
            print(f"Generating SVG for: {discussion['title']}")

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
                discussion_url=discussion['url']
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
