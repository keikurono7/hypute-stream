#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
OUTPUT_DIR="$SCRIPT_DIR/generated"
OUTPUT_CSV="$OUTPUT_DIR/amazon_reviews23.csv"
TARGET_RECORDS="${TARGET_RECORDS:-100000000}"
PYTHON_BIN="${PYTHON_BIN:-python3}"

mkdir -p "$OUTPUT_DIR"

existing_lines=0
if [[ -f "$OUTPUT_CSV" ]]; then
    existing_lines="$(wc -l < "$OUTPUT_CSV" 2>/dev/null || echo 0)"
fi

if [[ ! -f "$OUTPUT_CSV" ]] || [[ "$existing_lines" -lt 2 ]]; then
    echo "[HAYAKO] Amazon Reviews'23 benchmark CSV not found, generating from public 0-core rating-only files..."
    rm -f "$OUTPUT_CSV"
    "$PYTHON_BIN" - "$OUTPUT_CSV" "$TARGET_RECORDS" <<'PY'
import csv
import gzip
import hashlib
import io
import sys
import urllib.request

output_csv = sys.argv[1]
target_records = int(sys.argv[2])

sources = [
    ("Books",                      "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Books.csv.gz"),
    ("Home_and_Kitchen",           "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Home_and_Kitchen.csv.gz"),
    ("Electronics",                "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Electronics.csv.gz"),
    ("Clothing_Shoes_and_Jewelry", "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Clothing_Shoes_and_Jewelry.csv.gz"),
    ("Health_and_Household",       "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Health_and_Household.csv.gz"),
    ("Beauty_and_Personal_Care",   "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Beauty_and_Personal_Care.csv.gz"),
    ("Cell_Phones_and_Accessories","https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Cell_Phones_and_Accessories.csv.gz"),
    ("Automotive",                 "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Automotive.csv.gz"),
    ("Sports_and_Outdoors",        "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Sports_and_Outdoors.csv.gz"),
    ("Tools_and_Home_Improvement", "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Tools_and_Home_Improvement.csv.gz"),
    ("Pet_Supplies",               "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Pet_Supplies.csv.gz"),
    ("Patio_Lawn_and_Garden",      "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Patio_Lawn_and_Garden.csv.gz"),
    ("Toys_and_Games",             "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Toys_and_Games.csv.gz"),
    ("Grocery_and_Gourmet_Food",   "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Grocery_and_Gourmet_Food.csv.gz"),
    ("Unknown",                    "https://mcauleylab.ucsd.edu/public_datasets/data/amazon_2023/benchmark/0core/rating_only/Unknown.csv.gz"),
]


def stable_u64(value):
    digest = hashlib.blake2b(str(value).encode("utf-8"), digest_size=8).digest()
    return int.from_bytes(digest, "big", signed=False)


written = 0
with open(output_csv, "w", newline="") as handle:
    writer = csv.writer(handle)
    writer.writerow(["user_id", "item_id", "rating", "timestamp"])

    for category, url in sources:
        print(f"[HAYAKO] Streaming {category} from: {url}", flush=True)
        try:
            with urllib.request.urlopen(url, timeout=60) as response:
                with gzip.GzipFile(fileobj=response) as gz_stream:
                    text_stream = io.TextIOWrapper(gz_stream, encoding="utf-8")
                    reader = csv.DictReader(text_stream)

                    for row in reader:
                        user_id  = row.get("user_id")
                        item_id  = row.get("parent_asin")
                        rating   = row.get("rating")
                        timestamp = row.get("timestamp")

                        if not user_id or not item_id or rating is None or timestamp is None:
                            continue

                        writer.writerow([
                            stable_u64(user_id),
                            stable_u64(item_id),
                            float(rating),
                            int(float(timestamp)),
                        ])
                        written += 1

                        if written % 1000000 == 0:
                            print(f"[HAYAKO] Converted {written:,} rows...", flush=True)

                        if written >= target_records:
                            break

        except Exception as exc:
            print(f"[HAYAKO] Skipping {category}: {exc}", flush=True)
            continue

        if written >= target_records:
            break

if written == 0:
    raise SystemExit("[HAYAKO] No Amazon review rows were converted.")

print(f"[HAYAKO] Wrote {written:,} converted rows to {output_csv}", flush=True)
PY
else
    echo "[HAYAKO] Reusing existing benchmark CSV: $OUTPUT_CSV ($existing_lines lines)"
fi

if [[ ! -x "$REPO_ROOT/build/benchmark_runner" ]]; then
    cat <<EOF
[HAYAKO] Benchmark binary not found at $REPO_ROOT/build/benchmark_runner

Run these commands from the repository root:
  mkdir -p build
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j

Then run:
  ./build/benchmark_runner DEFAULT_CTX "$OUTPUT_CSV" "AMAZON REVIEWS'23 100M+"
EOF
    exit 0
fi

exec "$REPO_ROOT/build/benchmark_runner" DEFAULT_CTX "$OUTPUT_CSV" "AMAZON REVIEWS'23 100M+"
