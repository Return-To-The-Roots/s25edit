<!--
Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Map geometry alignment with s25client

The editor and `s25client` render the same staggered hexagonal grid, but the
indexing of **USD (up-side-down) triangles differs by one vertex in even rows**.
This document describes the mapping so that border / coastline generation can
be aligned with `libs/s25main/TerrainRenderer.cpp` while still using the
editor's `bobMAP` data.

## Vertex neighbour rules

For a vertex `pt = (x, y)` the neighbours used by `s25client` are:

| direction | even row (`y % 2 == 0`) | odd row (`y % 2 != 0`) |
|-----------|------------------------|------------------------|
| East      | `(x + 1, y)`           | `(x + 1, y)`           |
| SouthEast | `(x,     y + 1)`       | `(x + 1, y + 1)`       |
| SouthWest | `(x - 1, y + 1)`       | `(x,     y + 1)`       |

These are exactly the definitions from `s25client::GetNeighbour`.

## Triangle ownership

*`s25client` naming:*

- **RSU** at `(x, y)`: vertices `[pt, SW(pt), SE(pt)]` — points down.
- **USD** at `(x, y)`: vertices `[pt, SE(pt), E(pt)]` — points up.

*Editor naming (`bobMAP` / `CSurface::DrawTriangle`):*

- **RSU at `(x, y)`** uses the same three vertices as `s25client`, so the two
  agree completely.
- **USD at `(x, y)`** is shifted:
  - even `y`: editor USD `(x, y)` == `s25client` USD `(x - 1, y)`
  - odd  `y`: editor USD `(x, y)` == `s25client` USD `(x,     y)`

In compact form:

```text
editor USD(x, y) = s25client USD(x - !(y & 1), y)
s25client USD(x, y) = editor USD(x + !(y & 1), y)
```

## Texture lookups

Because of the USD shift, the texture attached to a rendered USD triangle is
fetched from a different `MapNode` index in the editor than in `s25client`:

- editor cached USD at `(x, y)` uses `map.getVertex(x - !(y & 1), y).usdTexture`
- `s25client` USD at `(x, y)` uses `map.getVertex(x, y).usdTexture`

For borders we therefore use `s25client`'s conceptual triangle grid but read
the editor's raw `MapNode` textures at the `s25client` vertex coordinates.
This keeps the coastline consistent with the terrain actually rendered by the
editor.

## Border generation mapping

`s25client::TerrainRenderer::GenerateOpenGL` decides per vertex `(x, y)` which
of six possible border triangles exist, using the four terrain samples:

```text
t1 = RSU(x, y)
t2 = USD(x, y)
t3 = RSU(x + 1, y)
t4 = USD(SW(x, y)) = USD(x - !(y & 1), y + 1)
```

Mapped to the editor's `bobMAP`:

```text
t1 = map.getVertex(x,         y).rsuTexture
t2 = map.getVertex(x,         y).usdTexture
t3 = map.getVertex(x + 1,     y).rsuTexture
t4 = map.getVertex(x - !(y&1), y + 1).usdTexture
```

The six border triangles (vertex positions and winding) are exactly those
from `s25client::TerrainRenderer::UpdateBorderTrianglePos`:

| border                         | condition                  | vertices                                   |
|--------------------------------|----------------------------|--------------------------------------------|
| `left_right[0]` (USD over RSU) | `GetEdgeType(t2, t1)`      | `[pt, SE(pt), centroid(RSU(x, y))]`       |
| `left_right[1]` (RSU over USD) | `GetEdgeType(t1, t2)`      | `[centroid(USD(x, y)), SE(pt), pt]`       |
| `right_left[0]` (RSU(E) over USD) | `GetEdgeType(t3, t2)`   | `[SE(pt), E(pt), centroid(USD(x, y))]`    |
| `right_left[1]` (USD over RSU(E)) | `GetEdgeType(t2, t3)`   | `[centroid(RSU(x+1, y)), E(pt), SE(pt)]` |
| `top_down[0]` (USD(SW) over RSU)  | `GetEdgeType(t4, t1)`   | `[SW(pt), SE(pt), centroid(RSU(x, y))]`   |
| `top_down[1]` (RSU over USD(SW))  | `GetEdgeType(t1, t4)`   | `[centroid(USD(SW)), SE(pt), SW(pt)]`     |

`centroid(RSU(x, y)) = (SW + pt + SE) / 3`  
`centroid(USD(x, y)) = (E + pt + SE) / 3`

Pixel positions are taken from `MapNode::x / y` and wrapped to stay adjacent to
the reference vertex, matching the wrap handling already used for terrain
vertices.

## Water animation

The editor animates water/lava by shifting texture coordinates in the
static/dynamic texcoord VBO, because the editor does not have the multiple
animated texture frames that `s25client` uses for palette-animated terrains.
The texcoord buffer is updated every animation frame, so it should be created
with `ogl::Usage::Dynamic` to match the old editor behaviour and avoid
implicit synchronisation stalls.
