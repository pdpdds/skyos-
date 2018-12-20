//
// Copyright 1999-2002 Jeff Bush
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef RASTERIZER_H
#define RASTERIZER_H

class Surface;

/// A rasterizer is a virtual base class for a class that can convert
/// geometric primitives into raster data.  A rasterizer is attached
/// to a surface.
class Rasterizer {
public:
	Rasterizer(Surface *surface)
		:	fSurface(surface)
	{
	}

	virtual void strokeLine(int x1, int y1, int x2, int y2, int color) = 0;
	virtual void fillRect(int x1, int y1, int x2, int y2, int color) = 0;

protected:
	inline Surface *getSurface();

private:
	Surface *fSurface;
};

inline Surface *Rasterizer::getSurface()
{
	return fSurface;
}

#endif
