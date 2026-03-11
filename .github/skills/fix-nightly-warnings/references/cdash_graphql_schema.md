# CDash GraphQL API Reference

## Overview

CDash exposes a GraphQL API at:

```
POST https://open.cdash.org/graphql
Content-Type: application/json
```

Example invocation:
```bash
curl -s -X POST https://open.cdash.org/graphql \
  -H 'Content-Type: application/json' \
  -d '{"query": "{ projects { edges { node { id name } } } }"}'
```

## Version Information

| Field              | Value                           |
|--------------------|---------------------------------|
| CDash version      | `v4.9.0-28-g6ffba1bef`         |
| Schema documented  | 2026-03-10                      |
| Endpoint           | `https://open.cdash.org/graphql` |
| ITK project ID     | `2` (name: `Insight`)           |

The version was retrieved from `https://open.cdash.org/api/v1/index.php?project=Insight`
(field `version` in the JSON response). The GraphQL schema itself has no version field
(`__schema { description }` returns null). If the scripts in `../scripts/` stop working,
check whether the CDash version has changed and re-introspect the schema:

```bash
curl -s -X POST https://open.cdash.org/graphql \
  -H 'Content-Type: application/json' \
  -d '{"query":"{ __schema { types { name kind } } }"}' | python3 -m json.tool
```

---

## Top-Level Query Fields

| Field            | Return Type              | Description                          |
|------------------|--------------------------|--------------------------------------|
| `me`             | `User`                   | Current authenticated user           |
| `project(id)`    | `Project`                | Single project by ID                 |
| `projects`       | `ProjectConnection`      | All visible projects                 |
| `build(id)`      | `Build`                  | Single build by ID                   |
| `site(id)`       | `Site`                   | Single site by ID                    |
| `buildCommand`   | `BuildCommand`           | Single build command by ID           |
| `dynamicAnalysis`| `DynamicAnalysis`        | Single dynamic analysis result       |

---

## Project Type

### `project(id: ID!): Project`

Key fields on `Project`:

| Field    | Type                | Description               |
|----------|---------------------|---------------------------|
| `id`     | `ID!`               | Project identifier        |
| `name`   | `String!`           | Project name              |
| `builds` | `BuildConnection!`  | Paginated list of builds  |

### `Project.builds` arguments

| Argument  | Type                                   | Description                   |
|-----------|----------------------------------------|-------------------------------|
| `filters` | `ProjectBuildsFiltersMultiFilterInput` | Filter combinators (see below) |
| `first`   | `Int!`                                 | Page size (required)          |
| `after`   | `String`                               | Cursor for pagination         |

### `ProjectBuildsFiltersMultiFilterInput` combinators

| Field     | Type                                   | Description                           |
|-----------|----------------------------------------|---------------------------------------|
| `any`     | `[ProjectBuildsFiltersMultiFilterInput]` | OR of child filters                 |
| `all`     | `[ProjectBuildsFiltersMultiFilterInput]` | AND of child filters                |
| `eq`      | `BuildFilterInput`                     | Field equals value                    |
| `ne`      | `BuildFilterInput`                     | Field not equals value                |
| `gt`      | `BuildFilterInput`                     | Field greater than value              |
| `lt`      | `BuildFilterInput`                     | Field less than value                 |
| `contains`| `BuildFilterInput`                     | Field contains substring              |
| `has`     | `BuildRelationshipFilterInput`         | Related entity filter                 |

### `BuildFilterInput` filterable fields

| Field                   | Type         |
|-------------------------|--------------|
| `id`                    | `ID`         |
| `name`                  | `String`     |
| `startTime`             | `DateTimeTz` |
| `endTime`               | `DateTimeTz` |
| `submissionTime`        | `DateTimeTz` |
| `stamp`                 | `String`     |
| `uuid`                  | `String`     |
| `generator`             | `String`     |
| `command`               | `String`     |
| `sourceDirectory`       | `String`     |
| `binaryDirectory`       | `String`     |
| `operatingSystemName`   | `String`     |
| `operatingSystemPlatform`| `String`    |
| `operatingSystemRelease`| `String`     |
| `operatingSystemVersion`| `String`     |
| `compilerName`          | `String`     |
| `compilerVersion`       | `String`     |

> **Note:** `buildWarningsCount`, `buildErrorsCount`, `buildType` are NOT filterable fields.
> Filter by `stamp` containing `"Nightly"` to get nightly builds, then filter warning counts client-side.

---

## Build Type

### `build(id: ID!): Build`

| Field                    | Type                   | Description                          |
|--------------------------|------------------------|--------------------------------------|
| `id`                     | `ID!`                  | Build identifier                     |
| `name`                   | `String!`              | Build name (e.g. `Mac26.x-ClangMain-dbg-arm64`) |
| `startTime`              | `DateTimeTz!`          | Build start time                     |
| `endTime`                | `DateTimeTz!`          | Build end time                       |
| `submissionTime`         | `DateTimeTz!`          | Submission time                      |
| `stamp`                  | `String!`              | Date-type stamp (e.g. `20260310-0100-Nightly`) |
| `buildType`              | `String!`              | e.g. `Nightly`, `Continuous`         |
| `configureErrorsCount`   | `Int`                  | Configure errors count               |
| `configureWarningsCount` | `Int`                  | Configure warnings count             |
| `buildErrorsCount`       | `Int`                  | Compile errors count                 |
| `buildWarningsCount`     | `Int`                  | Compile warnings count               |
| `failedTestsCount`       | `Int`                  | Failed tests count                   |
| `passedTestsCount`       | `Int`                  | Passed tests count                   |
| `site`                   | `Site!`                | Submitting site                      |
| `project`                | `Project!`             | Parent project                       |
| `buildErrors`            | `BuildErrorConnection!`| Warnings and errors (see below)      |
| `tests`                  | `TestConnection!`      | Test results                         |
| `configure`              | `Configure`            | Configure step                       |
| `updateStep`             | `Update`               | Update step                          |
| `coverage`               | `CoverageConnection!`  | Coverage results                     |

### `Build.buildErrors` arguments

| Argument  | Type                                     | Description                    |
|-----------|------------------------------------------|--------------------------------|
| `filters` | `BuildBuildErrorsFiltersMultiFilterInput`| Filter combinators (see below) |
| `first`   | `Int!`                                   | Page size (required)           |
| `after`   | `String`                                 | Cursor for pagination          |

### `BuildBuildErrorsFiltersMultiFilterInput` / `BuildErrorFilterInput`

Combinator fields: `any`, `all`, `eq`, `ne`, `gt`, `lt`, `contains`, `has`

Filterable fields via `BuildErrorFilterInput`:

| Field              | Type             | Notes                              |
|--------------------|------------------|------------------------------------|
| `id`               | `ID`             |                                    |
| `type`             | `BuildErrorType` | `WARNING` or `ERROR` — most useful |
| `sourceFile`       | `String`         |                                    |
| `stdOutput`        | `String`         |                                    |
| `stdError`         | `String`         |                                    |
| `workingDirectory` | `String`         |                                    |
| `exitCondition`    | `String`         |                                    |
| `language`         | `String`         |                                    |
| `targetName`       | `String`         |                                    |
| `outputFile`       | `String`         |                                    |
| `outputType`       | `String`         |                                    |
| `logLine`          | `Int`            |                                    |
| `sourceLine`       | `Int`            |                                    |

### `BuildErrorType` enum values

| Value     | Description          |
|-----------|----------------------|
| `ERROR`   | Compile error        |
| `WARNING` | Compile warning      |

---

## BuildError Type (node fields)

| Field              | Type                | Description                                   |
|--------------------|---------------------|-----------------------------------------------|
| `id`               | `ID!`               | Entry identifier                              |
| `type`             | `BuildErrorType!`   | `WARNING` or `ERROR`                          |
| `sourceFile`       | `String!`           | Source file path relative to source dir       |
| `sourceLine`       | `Int`               | Line number in source file                    |
| `stdOutput`        | `String!`           | Compiler stdout for this entry                |
| `stdError`         | `String!`           | Compiler stderr for this entry (warning text) |
| `workingDirectory` | `String!`           | Working directory during compilation          |
| `exitCondition`    | `String!`           | Exit code/condition string                    |
| `language`         | `String!`           | Language (`C`, `CXX`, etc.)                   |
| `targetName`       | `String!`           | CMake target name                             |
| `outputFile`       | `String!`           | Object file path                              |
| `outputType`       | `String!`           | Output type                                   |
| `logLine`          | `Int`               | Line in full build log                        |
| `command`          | `String!`           | Full compiler command line                    |
| `labels`           | `LabelConnection!`  | Labels attached to this entry                 |

> **Tip:** The `-Wfoo` warning flag is embedded in `stdError`, not a separate structured field.
> Parse it with a regex: `re.search(r'\[(-W[^\]]+)\]', node['stdError'])`.

---

## Common Query Patterns

### List recent nightly builds with warning counts

```graphql
{
  project(id: "2") {
    builds(first: 50, filters: { contains: { stamp: "Nightly" } }) {
      edges {
        node {
          id name stamp buildWarningsCount buildErrorsCount
          startTime site { name }
        }
      }
    }
  }
}
```

Filter builds with `buildWarningsCount > 0` **client-side** (not filterable server-side).

### Get all warnings for a specific build

```graphql
{
  build(id: "11107692") {
    name site { name }
    buildWarningsCount
    buildErrors(filters: { eq: { type: WARNING } }, first: 200) {
      edges {
        node { sourceFile sourceLine stdError stdOutput }
      }
    }
  }
}
```

### Paginate through large result sets

Use `pageInfo` from the connection and the `after` cursor argument:

```graphql
{
  build(id: "11107692") {
    buildErrors(filters: { eq: { type: WARNING } }, first: 50, after: "CURSOR") {
      pageInfo { hasNextPage endCursor }
      edges { node { sourceFile sourceLine stdError } }
    }
  }
}
```
