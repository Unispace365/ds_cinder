Cluster Rendering guide
====================

This document was placed here to guide you how to set up `engine.xml` for a clustered setup.  DS Cinder has a client-server architecture and clients can either render a **continuous** portion of the world or a **discontinuous** portion of the world. This guide will walk you through the differences and their setup requirements:

Continuous vs. Discontinuous
----------------------------

Terminology refers to the fact that a client instance attempts to render a continuous block of the world or not. Essentially, continuous block means rendering **ONE AND ONLY ONE** part of the world into **ONE AND ONLY ONE** part of the client window. Discontinuous means the client attempts to render multiple distinct parts of the world into multiple distinct parts of the client window.

Continuous Cluster Rendering
----------------------------

For this setup you need:

 1. One `world_dimensions` size. (the entire world)
 2. One `src_rect` (the ONLY part of the world you are trying to render)
 3. One `dst_rect` (the ONLY part of the client window that will show `src_rect`'s content)

Example setup would be:

```
<settings>
	<!-- how big the world size is, in pixels -->
	<size name="world_dimensions" x="5760" y="1080" />
	<!-- part of the world we want to render, in pixels -->
	<rect name="src_rect" l="1920" r="3840" t="0" b="1080" />
	<!-- the client window that src_rect will be rendered into -->
	<rect name="dst_rect" l="1920" r="3840" t="0" b="1080" />
</settings>
```
Discontinuous Cluster Rendering
----------------------------

For this setup you need:

 1. One `world_dimensions` size. (the entire world)
 2. Many `src_rect`s (the parts of the world you are trying to render)
 3. Many `dst_rect`s (the parts of the client window that will show `src_rect`'s content)
 4. One `screen_rect` that will contain the final result in one window.

Do note that there's a one-to-one mapping between `src_rect`s and `dst_rect`s. Each `dst_rect` renders its equivalent `src_rect` in `src_rect` stack. Also do note, this mode does not work with FXAA.

Example setup would be:

```
<settings>
	<!-- how big the world size is, in pixels -->
	<size name="world_dimensions" x="5760" y="1080" />
	
	<!-- LEFT part of the world we want to render, in pixels -->
	<rect name="src_rect" l="0" r="1920" t="0" b="1080" />
	<rect name="dst_rect" l="0" r="1920" t="0" b="1080" />

	<!-- RIGHT part of the world we want to render, in pixels -->
	<rect name="src_rect" l="3840" r="5760" t="0" b="1080" />
	<rect name="dst_rect" l="3840" r="5760" t="0" b="1080" />
	
	<rect name="screen_rect" l="0" r="5760" t="0" b="1080" />
</settings>
```
