# Amazon Reviews'23 Benchmark

This benchmark path streams Amazon Reviews'23 data from Hugging Face, converts the review records into the numeric CSV shape expected by the existing benchmark runner, and then runs the comparison binary.

## Run

From the repository root:

```bash
chmod +x benchmarks/amazon_reviews23/setup_and_run.sh
TARGET_RECORDS=100000000 ./benchmarks/amazon_reviews23/setup_and_run.sh
```

## Notes

The converter uses `user_id` and `parent_asin` from the review rows, hashes them into stable numeric identifiers, and keeps `rating` plus `timestamp` in the output CSV.

The default configuration streams a large set of review domains that should exceed 100M rows when fully processed. You can override the list with `AMAZON_REVIEW_CONFIGS` if you want to narrow or expand the source set.

If `datasets` is missing, install it with:

```bash
python3 -m pip install datasets
```