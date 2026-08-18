# zephyr-workspace

West workspace for this project. `app` is the manifest repository
(`app/west.yml`); all modules that west pulls in (`zephyr`, `modules/*`, ...)
are cloned as siblings of `app`, not nested inside it.

## Initial setup

From this directory:

```sh
west init -l app
west update
```

- `west init -l app` — initializes the workspace using `app/west.yml` as the
  manifest (local import), creating `.west/` here in `zephyr-workspace`.
- `west update` — clones/updates the manifest projects as siblings of `app`
  (e.g. `zephyr-workspace/zephyr`, `zephyr-workspace/modules/...`). These are
  gitignored since they're reproducible from the manifest.
