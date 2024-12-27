from github import Github
import os

GITHUB_TOKEN = os.environ['GITHUB_TOKEN']
if not GITHUB_TOKEN:
    raise ValueError("GITHUB_TOKEN environment variable is not set")

def generate_github_discussion_svg(title, username, emoji, labels, category, upvotes, comments):
    width = 820
    height = 120
    emoji_size = 20 # 16 font size equal 20x19 px
    emoji_box_x = 30
    emoji_box_size = 54
    upvote_width = len(str(upvotes)) * 10 + 30
    upvote_x_center = 760
    upvote_rect_x = upvote_x_center - upvote_width / 2

    # Create SVG content
    svg = f"""
    <svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">
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
        <text x="100" y="40" font-size="20" fill="#9198a1" font-family="Arial" font-weight="bold">{title}</text>

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
        <text x="100" y="95" font-size="14" fill="#9198a1" font-family="Arial">{username} started this discussion</text>

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

def fetch_recent_discussions():
    g = Github(GITHUB_TOKEN)

    #query = """
    #{
    #  repository(owner: "brenocq", name: "implot3d") {
    #    discussions(first: 5, orderBy: {field: UPDATED_AT, direction: DESC}) {
    #      nodes {
    #        title
    #        url
    #        updatedAt
    #      }
    #    }
    #  }
    #}
    #"""

    #result = g.graphql(query)
    #discussions = result["data"]["repository"]["discussions"]["nodes"]
    #for discussion in discussions:
    #    print(f"{discussion['title']} - {discussion['url']} (Updated at: {discussion['updatedAt']})")
    for repo in g.get_user().get_repos():
        print(repo.name)
        repo.edit(has_wiki=False)
        # to see all the available attributes and methods
        if repo.name == "implot3d":
            print(dir(repo))

    g.close()

fetch_recent_discussions()

# Example usage
labels = [
    {"text": "type:chore", "color": "#0366d6"},
    {"text": "prio:medium", "color": "#f1c40f"},
    {"text": "status:todo", "color": "#3498db"}
]

svg_output = generate_github_discussion_svg(
    title="Hosted online demo",
    username="brenocq",
    emoji="💡",
    labels=labels,
    category="Features and improvements",
    upvotes=3,
    comments=2
)

# Save to file
with open("discussion_card.svg", "w") as f:
    f.write(svg_output)

fetch_recent_discussions()
