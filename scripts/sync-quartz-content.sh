#!/usr/bin/env bash
set -euo pipefail

VAULT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 /absolute/path/to/quartz"
  exit 64
fi

if [[ "$1" != /* ]]; then
  echo "Quartz path must be absolute: $1"
  exit 64
fi

if [[ ! -d "$1" ]]; then
  echo "Quartz directory does not exist: $1"
  exit 66
fi

QUARTZ_DIR="$(cd "$1" && pwd -P)"
CONTENT_DIR="$QUARTZ_DIR/content"

if [[ ! -f "$QUARTZ_DIR/quartz.config.ts" || ! -f "$QUARTZ_DIR/package.json" ]]; then
  echo "Not a Quartz project: $QUARTZ_DIR"
  exit 65
fi

if [[ "$CONTENT_DIR" == "/" || -z "$CONTENT_DIR" ]]; then
  echo "Refusing unsafe content directory: $CONTENT_DIR"
  exit 65
fi

STAGING_DIR="$(mktemp -d "${TMPDIR:-/tmp}/my-study-quartz.XXXXXX")"
trap 'rm -rf "$STAGING_DIR"' EXIT

RSYNC_EXCLUDES=(
  --exclude='/.git/***'
  --exclude='/.obsidian/***'
  --exclude='/.vscode/***'
  --exclude='/.DS_Store'
  --exclude='/README.md'
)

# Avoid copying Quartz back into itself if it lives inside this vault.
case "$QUARTZ_DIR/" in
  "$VAULT_DIR/"*)
    QUARTZ_RELATIVE="${QUARTZ_DIR#"$VAULT_DIR/"}"
    RSYNC_EXCLUDES+=(--exclude="/$QUARTZ_RELATIVE/***")
    ;;
esac

rsync -a --prune-empty-dirs \
  "${RSYNC_EXCLUDES[@]}" \
  --include='*/' \
  --include='*.md' \
  --exclude='*' \
  "$VAULT_DIR/" \
  "$STAGING_DIR/"

# Preserve each document's last Git modification date in generated frontmatter.
# This keeps Quartz dates stable even though content is copied into a build folder.
while IFS= read -r -d '' DOCUMENT_PATH; do
  DOCUMENT_RELATIVE="${DOCUMENT_PATH#"$STAGING_DIR/"}"
  MODIFIED_DATE="$(git -C "$VAULT_DIR" log -1 --format=%cs -- "$DOCUMENT_RELATIVE")"
  MODIFIED_DATE="${MODIFIED_DATE:-$(date '+%Y-%m-%d')}"
  DOCUMENT_TEMP="$DOCUMENT_PATH.tmp"

  if [[ "$(sed -n '1p' "$DOCUMENT_PATH")" == "---" ]]; then
    if awk '
      NR == 1 && $0 == "---" { in_frontmatter = 1; next }
      in_frontmatter && $0 == "---" { exit !found }
      in_frontmatter && /^modified:[[:space:]]*/ { found = 1 }
      END { if (in_frontmatter) exit !found }
    ' "$DOCUMENT_PATH"; then
      continue
    fi

    {
      printf '%s\n' '---' "modified: $MODIFIED_DATE"
      sed '1d' "$DOCUMENT_PATH"
    } >"$DOCUMENT_TEMP"
  else
    {
      printf '%s\n' '---' "modified: $MODIFIED_DATE" '---' ''
      cat "$DOCUMENT_PATH"
    } >"$DOCUMENT_TEMP"
  fi

  mv "$DOCUMENT_TEMP" "$DOCUMENT_PATH"
done < <(find "$STAGING_DIR" -type f -name '*.md' -print0)

# Use the vault README as the homepage. Add Quartz frontmatter and avoid
# rendering the README's H1 twice on the generated page.
README_MODIFIED_DATE="$(git -C "$VAULT_DIR" log -1 --format=%cs -- README.md)"
README_MODIFIED_DATE="${README_MODIFIED_DATE:-$(date '+%Y-%m-%d')}"
{
  printf '%s\n' \
    '---' \
    'title: 정명진의 개발 노트' \
    "modified: $README_MODIFIED_DATE" \
    '---' \
    ''
  sed '1d' "$VAULT_DIR/README.md"
} >"$STAGING_DIR/index.md"

mkdir -p "$CONTENT_DIR"
rsync -a --delete "$STAGING_DIR/" "$CONTENT_DIR/"

NON_MARKDOWN_COUNT="$(find "$CONTENT_DIR" -type f ! -name '*.md' | wc -l | tr -d ' ')"
if [[ "$NON_MARKDOWN_COUNT" != "0" ]]; then
  echo "Sync failed: non-Markdown files exist in $CONTENT_DIR"
  exit 1
fi

DOCUMENT_COUNT="$(find "$CONTENT_DIR" -type f -name '*.md' | wc -l | tr -d ' ')"
echo "Synced $DOCUMENT_COUNT Markdown documents to $CONTENT_DIR"
