//
// Simple Wavefront .obj parser in C
// Anton Gerdelan 22 Dec 2014
// antongerdelan.net
//
#include "obj_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

bool load_obj_file  (const char* file_name, float** points, float** tex_coords,
	float** normals, int* point_count) {
	FILE* fp = NULL;
	float* unsorted_vp_array = NULL;
	float* unsorted_vt_array = NULL;
	float* unsorted_vn_array = NULL;
	int current_unsorted_vp = 0;
	int current_unsorted_vt = 0;
	int current_unsorted_vn = 0;
	int unsorted_vp_count = 0;
	int unsorted_vt_count = 0;
	int unsorted_vn_count = 0;
	int face_count = 0;
	int i;
	char line[1024];

	fp = fopen (file_name, "r");
	if (!fp) {
		fprintf (stderr, "ERROR: could not find file %s\n", file_name);
		return false;
	}

	// first count points in file so we know how much mem to allocate
	*point_count = 0;
	while (fgets (line, 1024, fp)) {
		if (line[0] == 'v') {
			if (line[1] == ' ') {
				unsorted_vp_count++;
			} else if (line[1] == 't') {
				unsorted_vt_count++;
			} else if (line[1] == 'n') {
				unsorted_vn_count++;
			}
		} else if (line[0] == 'f') {
			face_count++;
		}
	}
	printf ("found %i vp %i vt %i vn unique in obj. allocating memory...\n",
		unsorted_vp_count, unsorted_vt_count, unsorted_vn_count);
	unsorted_vp_array = (float*)malloc (unsorted_vp_count * 3 * sizeof (float));
	unsorted_vt_array = (float*)malloc (unsorted_vt_count * 2 * sizeof (float));
	unsorted_vn_array = (float*)malloc (unsorted_vn_count * 3 * sizeof (float));
	*points = (float*)malloc (3 * face_count * 3 * sizeof (float));
	*tex_coords = (float*)malloc (3 * face_count * 2 * sizeof (float));
	*normals = (float*)malloc (3 * face_count * 3 * sizeof (float));
	printf ("allocated %i bytes for mesh\n", (int)(3 * face_count * 8 *
		sizeof (float)));

	rewind (fp);
	while (fgets (line, 1024, fp)) {
		// vertex
		if (line[0] == 'v') {

			// vertex point
			if (line[1] == ' ') {
				float x, y, z;
				x = y = z = 0.0f;
				sscanf (line, "v %f %f %f", &x, &y, &z);
				unsorted_vp_array[current_unsorted_vp * 3] = x;
				unsorted_vp_array[current_unsorted_vp * 3 + 1] = y;
				unsorted_vp_array[current_unsorted_vp * 3 + 2] = z;
				current_unsorted_vp++;

			// vertex texture coordinate
			} else if (line[1] == 't') {
				float s, t;
				s = t = 0.0f;
				sscanf (line, "vt %f %f", &s, &t);
				unsorted_vt_array[current_unsorted_vt * 2] = s;
				unsorted_vt_array[current_unsorted_vt * 2 + 1] = t;
				current_unsorted_vt++;

			// vertex normal
			} else if (line[1] == 'n') {
				float x, y, z;
				x = y = z = 0.0f;
				sscanf (line, "vn %f %f %f", &x, &y, &z);
				unsorted_vn_array[current_unsorted_vn * 3] = x;
				unsorted_vn_array[current_unsorted_vn * 3 + 1] = y;
				unsorted_vn_array[current_unsorted_vn * 3 + 2] = z;
				current_unsorted_vn++;
			}

		// faces
		} else if (line[0] == 'f') {
			int vp[3], vt[3], vn[3], len;
			int slashCount = 0;
			// work out if using quads instead of triangles and print a warning

			len = strlen (line);
			for (i = 0; i < len; i++) {
				if (line[i] == '/') {
					slashCount++;
				}
			}
			if (slashCount != 6) {
				fprintf (
					stderr,
					"ERROR: file contains quads or does not match v vp/vt/vn layout - \
					make sure exported mesh is triangulated and contains vertex points, \
					texture coordinates, and normals\n"
				);
				return false;
			}

			sscanf (line, "f %i/%i/%i %i/%i/%i %i/%i/%i",
			 &vp[0], &vt[0], &vn[0], &vp[1], &vt[1], &vn[1], &vp[2], &vt[2], &vn[2]);
			/* start reading points into a buffer. order is -1 because obj starts
			   from 1, not 0 */
			// NB: assuming all indices are valid
			for (i = 0; i < 3; i++) {
				int pc = *point_count;
				if ((vp[i] - 1 < 0) || (vp[i] - 1 >= unsorted_vp_count)) {
					fprintf (stderr, "ERROR: invalid vertex position index in face\n");
					return false;
				}
				if ((vt[i] - 1 < 0) || (vt[i] - 1 >= unsorted_vt_count)) {
					fprintf (stderr, "ERROR: invalid texture coord index %i in face.\n",
						vt[i]);
					return false;
				}
				if ((vn[i] - 1 < 0) || (vn[i] - 1 >= unsorted_vn_count)) {
					printf ("ERROR: invalid vertex normal index in face\n");
					return false;
				}
				// note - parentheses needed for C array dereferencing w/ptr
				(*points)[pc * 3] = unsorted_vp_array[(vp[i] - 1) * 3];
				(*points)[pc * 3 + 1] = unsorted_vp_array[(vp[i] - 1) * 3 + 1];
				(*points)[pc * 3 + 2] = unsorted_vp_array[(vp[i] - 1) * 3 + 2];
				(*tex_coords)[pc * 2] = unsorted_vt_array[(vt[i] - 1) * 2];
				(*tex_coords)[pc * 2 + 1] = unsorted_vt_array[(vt[i] - 1) * 2 + 1];
				(*normals)[pc * 3] = unsorted_vn_array[(vn[i] - 1) * 3];
				(*normals)[pc * 3 + 1] = unsorted_vn_array[(vn[i] - 1) * 3 + 1];
				(*normals)[pc * 3 + 2] = unsorted_vn_array[(vn[i] - 1) * 3 + 2];
				// ++ doesnt work here for deref, need brackets or it increments ptr
				(*point_count)++;
			}
		}
	}
	fclose (fp);
	free (unsorted_vp_array);
	free (unsorted_vn_array);
	free (unsorted_vt_array);
	printf ("allocated %i points\n", *point_count);
	return true;
}
