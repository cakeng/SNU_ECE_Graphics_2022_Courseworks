#include "class.h"

object* obj_init()
{
	object* newobj = new object();
	newobj->v_list = new std::vector<vertex*>();
	newobj->e_list = new std::vector<edge*>();
	newobj->f_list = new std::vector<face*>();
	newobj->vertices = new std::vector<GLfloat>();
	newobj->vertexIndices = new std::vector<unsigned int>();
	return newobj;
}

vertex* vtx_init()
{
	vertex* newvtx = new vertex();
	newvtx->e_list = new std::vector<edge*>();
	newvtx->f_list = new std::vector<face*>();
	newvtx->v_new = NULL;
	newvtx->idx = -1;
	return newvtx;
}

edge* edge_init()
{
	edge* newedge = new edge();
	newedge->f_list = new std::vector<face*>();
	newedge->v1 = NULL;
	newedge->v2 = NULL;
	newedge->edge_pt = NULL;
	return newedge;
}

face* face_init()
{
	face* newface = new face();
	newface->v_list = new std::vector<vertex*>();
	newface->e_list = new std::vector<edge*>();
	newface->face_pt = NULL;
	return newface;
}

vertex* add_vertex(object* obj, const coord& coord)
{
	vertex* newvtx = vtx_init();
	newvtx->xyz.x = coord.x;
	newvtx->xyz.y = coord.y;
	newvtx->xyz.z = coord.z;
	newvtx->idx = obj->v_list->size();
	obj->v_list->push_back(newvtx);
	return newvtx;
}

vertex* find_vertex(object* obj, const coord& coord)
{
	std::vector<vertex*>* vtxList = obj->v_list;
	for(int i = 0; i < vtxList->size(); i++)
	{
		if((*vtxList)[i]->xyz.x == coord.x && (*vtxList)[i]->xyz.y == coord.y && (*vtxList)[i]->xyz.z == coord.z)
		{
			return (*vtxList)[i];
		}
	}
	return NULL;
}

vertex* find_if_not_add_vertex (object* obj, const coord& coord)
{
	vertex *target = find_vertex (obj, coord);
	if (!target) target = add_vertex (obj, coord);
	return target;
}

edge* add_edge(object* obj, vertex* v1, vertex* v2)
{
	edge* newedge = edge_init();
	newedge->v1 = v1;
	newedge->v2 = v2;
	v1->e_list->push_back(newedge);
	v2->e_list->push_back(newedge);
	obj->e_list->push_back(newedge);
	return newedge;
}

edge* find_edge(object* obj, vertex* v1, vertex* v2)
{
	std::vector<edge*>* v1_edgeList = v1->e_list;
	for(int i = 0; i < v1_edgeList->size(); i++)
	{
		if((*v1_edgeList)[i]->v1 == v2 || (*v1_edgeList)[i]->v2 == v2)
		{
			return (*v1_edgeList)[i];
		}
	}
	return NULL;
}

edge* find_if_not_add_edge (object* obj, vertex* v1, vertex* v2)
{
	edge *target = find_edge (obj, v1, v2);
	if (!target) target = add_edge (obj, v1, v2);
	return target;
}

face* add_face(object* obj, const std::vector<int>& vertexIndices)
{
	face* newface = face_init();
	int n = vertexIndices.size();
	for (int i = 0; i < n; i++)
	{
		vertex* v1 = (*(obj->v_list))[vertexIndices[i]];
		vertex* v2 = (*(obj->v_list))[vertexIndices[(i+1)%n]];
		v1->f_list->push_back(newface);

		edge* temp = find_edge(obj, v1, v2);
		if(!temp) temp = add_edge(obj, v1, v2);

		temp->f_list->push_back(newface);
		newface->e_list->push_back(temp);
		newface->v_list->push_back(v1);
	}
	obj->f_list->push_back(newface);
	return newface;
}

coord add(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.x + ord2.x;
	temp.y = ord1.y + ord2.y;
	temp.z = ord1.z + ord2.z;
	return temp;
}

coord sub(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.x - ord2.x;
	temp.y = ord1.y - ord2.y;
	temp.z = ord1.z - ord2.z;
	return temp;
}

coord mul(const coord& ord1, GLfloat m)
{
	coord temp;
	temp.x = ord1.x * m;
	temp.y = ord1.y * m;
	temp.z = ord1.z * m;
	return temp;
}

coord div(const coord& ord1, GLfloat d)
{
	coord temp;
	temp.x = ord1.x / d;
	temp.y = ord1.y / d;
	temp.z = ord1.z / d;
	return temp;
}

coord cross(const coord& ord1, const coord& ord2)
{
	coord temp;
	temp.x = ord1.y * ord2.z - ord1.z * ord2.y;
	temp.y = ord1.z * ord2.x - ord1.x * ord2.z;
	temp.z = ord1.x * ord2.y - ord1.y * ord2.x;
	return temp;
}

void setNorm(object* obj)
{
	for (int i = 0; i < obj->f_list->size(); i++)
	{
		face* temp = (*(obj->f_list))[i];
		coord v01 = sub((*(temp->v_list))[1]->xyz, (*(temp->v_list))[0]->xyz);
		coord v12 = sub((*(temp->v_list))[2]->xyz, (*(temp->v_list))[1]->xyz);
		coord crs = cross(v01, v12);
		crs.normalize();
		temp->norm = crs;
	}

	for (int i = 0; i < obj->v_list->size(); i++)
	{
		coord sum;
		std::vector<face*>* temp = (*(obj->v_list))[i]->f_list;
		int n = temp->size();
		for (int j = 0; j < n; j++)
		{
			sum.add((*temp)[j]->norm);
		}
		sum.div((GLfloat)n);
		sum.normalize();
		(*(obj->v_list))[i]->avg_norm = sum;
	}
}

void aggregate_vertices(object* obj)
{
	obj->vertices->clear();
	obj->vertexIndices->clear();

	for (int i = 0; i < obj->v_list->size(); i++)
	{
		coord temp_pos = (*(obj->v_list))[i]->xyz;
		coord temp_norm = (*(obj->v_list))[i]->avg_norm;
		obj->vertices->push_back(temp_pos.x);
		obj->vertices->push_back(temp_pos.y);
		obj->vertices->push_back(temp_pos.z);
		obj->vertices->push_back(temp_norm.x);
		obj->vertices->push_back(temp_norm.y);
		obj->vertices->push_back(temp_norm.z);
	}
	if (obj->vertices_per_face == 3)
	{
		for (int i = 0; i < obj->f_list->size(); i++)
		{
			std::vector<vertex*>* temp = (*(obj->f_list))[i]->v_list;
			obj->vertexIndices->push_back((*temp)[0]->idx);
			obj->vertexIndices->push_back((*temp)[1]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
		}
	}
	else if (obj->vertices_per_face == 4)
	{
		for (int i = 0; i < obj->f_list->size(); i++)
		{
			std::vector<vertex*>* temp = (*(obj->f_list))[i]->v_list;
			obj->vertexIndices->push_back((*temp)[0]->idx);
			obj->vertexIndices->push_back((*temp)[1]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
			obj->vertexIndices->push_back((*temp)[2]->idx);
			obj->vertexIndices->push_back((*temp)[3]->idx);
			obj->vertexIndices->push_back((*temp)[0]->idx);
		}
	}
}

object* cube()
{
	object* newobj = obj_init();
	newobj->vertices_per_face = 4;
	for (int x = -1; x <= 1; x += 2)
	{
		for (int y = -1; y <= 1; y += 2)
		{
			for (int z = -1; z <= 1; z += 2)
			{
				add_vertex(newobj, coord((GLfloat)x, (GLfloat)y, (GLfloat)z));
			}
		}
	}
	add_face(newobj, { 0,2,6,4 });
	add_face(newobj, { 0,4,5,1 });
	add_face(newobj, { 0,1,3,2 });
	add_face(newobj, { 2,3,7,6 });
	add_face(newobj, { 6,7,5,4 });
	add_face(newobj, { 1,5,7,3 });

	setNorm(newobj);

	aggregate_vertices(newobj);

	return newobj;
}


object* donut()
{
	object* m = obj_init();
	m->vertices_per_face = 4;
	int i;
	coord v[] = {
		{ -2, -.5, -2 }, { -2, -.5,  2 }, {  2, -.5, -2 }, {  2, -.5,  2 },
		{ -1, -.5, -1 }, { -1, -.5,  1 }, {  1, -.5, -1 }, {  1, -.5,  1 },
		{ -2,  .5, -2 }, { -2,  .5,  2 }, {  2,  .5, -2 }, {  2,  .5,  2 },
		{ -1,  .5, -1 }, { -1,  .5,  1 }, {  1,  .5, -1 }, {  1,  .5,  1 },
	};

	for (i = 0; i < 16; i++) add_vertex(m, coord(v[i].x, v[i].y, v[i].z));
	add_face(m, { 4, 5, 1, 0 });
	add_face(m, { 3, 1, 5, 7 });
	add_face(m, { 0, 2, 6, 4 });
	add_face(m, { 2, 3, 7, 6 });

	add_face(m, { 8, 9, 13, 12 });
	add_face(m, { 15, 13, 9, 11 });
	add_face(m, { 12, 14, 10, 8 });
	add_face(m, { 14, 15, 11, 10 });

	add_face(m, { 0, 1, 9, 8 });
	add_face(m, { 1, 3, 11, 9 });
	add_face(m, { 2, 0, 8, 10 });
	add_face(m, { 3, 2, 10, 11 });

	add_face(m, { 12, 13, 5, 4 });
	add_face(m, { 13, 15, 7, 5 });
	add_face(m, { 14, 12, 4, 6 });
	add_face(m, { 15, 14, 6, 7 });

	setNorm(m);

	aggregate_vertices(m);

	return m;
}

object* star()
{
	object* m = obj_init();
	m->vertices_per_face = 3;
	int ang, i;
	double rad;
	coord v[15];

	for (i = 0; i < 5; i++) {
		ang = i * 72;
		rad = ang * 3.1415926 / 180;
		v[i].x = 2.2 * cos(rad); v[i].y = 2.2 * sin(rad); v[i].z = 0;

		rad = (ang + 36) * 3.1415926 / 180;
		v[i + 5].x = v[i + 10].x = cos(rad);
		v[i + 5].y = v[i + 10].y = sin(rad);
		v[i + 5].z = .5;
		v[i + 10].z = -.5;
	}

	for (i = 0; i < 15; i++) add_vertex(m, coord(v[i].x, v[i].y, v[i].z));
	add_face(m, { 0, 5, 9 });  // 0 1 2
	add_face(m, { 1, 6, 5 });  // 3 4 1
	add_face(m, { 2, 7, 6 });  // 5 6 4
	add_face(m, { 3, 8, 7 });  // 7 8 6
	add_face(m, { 4, 9, 8 });  // 9 2 8

	add_face(m, { 0, 14, 10 });// 0 10 11
	add_face(m, { 1, 10, 11 });// 3 11 12
	add_face(m, { 2, 11, 12 });// 5 12 13
	add_face(m, { 3, 12, 13 });// 7 13 14
	add_face(m, { 4, 13, 14 });// 9 14 10

	add_face(m, { 0, 10, 5 });// 0 11 1
	add_face(m, { 1, 5, 10 });// 3 1 11
	add_face(m, { 1, 11, 6 });// 3 12 4
	add_face(m, { 2, 6, 11 });
	add_face(m, { 2, 12, 7 });
	add_face(m, { 3, 7, 12 });
	add_face(m, { 3, 13, 8 });
	add_face(m, { 4, 8, 13 });
	add_face(m, { 4, 14, 9 });
	add_face(m, { 0, 9, 14 });

	setNorm(m);

	aggregate_vertices(m);

	return m;
}

bool is_holeEdge(edge* e)
{
	/* fill in the blank */
	return e->f_list->size() == 1; // delete this line after you fill in the blank.
}

bool is_holeVertex(vertex* v)
{
	/* fill in the blank */
	return v->e_list->size() != v->f_list->size(); // delete this line after you fill in the blank.
}

vertex* face_point(face* f)
{
	if (f->face_pt == NULL)
	{
		f->face_pt = vtx_init();
		f->face_pt->xyz.x = 0.0f;
		f->face_pt->xyz.y = 0.0f;
		f->face_pt->xyz.z = 0.0f;
		for (int i = 0; i < f->v_list->size(); i++)
		{
			f->face_pt->xyz.add((*(f->v_list))[i]->xyz);
		}
		f->face_pt->xyz.div(f->v_list->size());
	}
	//printf ("Face point (%3.3f, %3.3f, %3.3f) from vertice ", f->face_pt->xyz.x, f->face_pt->xyz.y, f->face_pt->xyz.z);
	for (int i = 0; i < f->v_list->size(); i++)
	{
		//printf ("(%3.3f, %3.3f, %3.3f) ", (*(f->v_list))[i]->xyz.x, (*(f->v_list))[i]->xyz.y, (*(f->v_list))[i]->xyz.z);
	}
	//printf ("\n");
	return f->face_pt; // delete this line after you fill in the blank.
}

vertex* edge_point(edge* e)
{
	if (e->edge_pt == NULL)
	{
		if (is_holeEdge(e))
		{
			e->edge_pt = vtx_init();
			e->edge_pt->xyz.x = 0.0f;
			e->edge_pt->xyz.y = 0.0f;
			e->edge_pt->xyz.z = 0.0f;
			e->edge_pt->xyz.add (e->v1->xyz);
			e->edge_pt->xyz.add (e->v2->xyz);
			e->edge_pt->xyz.div (2.0f);
			//printf ("Edge point (%3.3f, %3.3f, %3.3f) from vertice ", e->edge_pt->xyz.x, e->edge_pt->xyz.y, e->edge_pt->xyz.z);
			//printf ("(%3.3f, %3.3f, %3.3f) ", e->v1->xyz.x, e->v1->xyz.y, e->v1->xyz.z);
			//printf ("(%3.3f, %3.3f, %3.3f) ", e->v2->xyz.x, e->v2->xyz.y, e->v2->xyz.z);
			//printf ("\n");
		}
		else
		{
			e->edge_pt = vtx_init();
			e->edge_pt->xyz.x = 0.0f;
			e->edge_pt->xyz.y = 0.0f;
			e->edge_pt->xyz.z = 0.0f;
			e->edge_pt->xyz.add (e->v1->xyz);
			e->edge_pt->xyz.add (e->v2->xyz);
			for (int i = 0; i < e->f_list->size(); i++)
			{
				e->edge_pt->xyz.add (face_point((*(e->f_list))[i])->xyz);
			}
			e->edge_pt->xyz.div (2.0f + e->f_list->size());
			//printf ("Edge point (%3.3f, %3.3f, %3.3f) from vertice ", e->edge_pt->xyz.x, e->edge_pt->xyz.y, e->edge_pt->xyz.z);
			//printf ("(%3.3f, %3.3f, %3.3f) ", e->v1->xyz.x, e->v1->xyz.y, e->v1->xyz.z);
			//printf ("(%3.3f, %3.3f, %3.3f) ", e->v2->xyz.x, e->v2->xyz.y, e->v2->xyz.z);
			for (int i = 0; i < e->f_list->size(); i++)
			{
				//printf ("(%3.3f, %3.3f, %3.3f) ", (*(e->f_list))[i]->face_pt->xyz.x, (*(e->f_list))[i]->face_pt->xyz.y, (*(e->f_list))[i]->face_pt->xyz.z);
			}
			//printf ("\n");
		}
	}
	return e->edge_pt; // delete this line after you fill in the blank.
}

vertex* vertex_point(vertex* v)
{
	/* fill in the blank */
	if (v->v_new == NULL)
	{
		if (is_holeVertex(v))
		{
			v->v_new = vtx_init();
			v->v_new->xyz.x = 0.0f;
			v->v_new->xyz.y = 0.0f;
			v->v_new->xyz.z = 0.0f;
			int count = 0;
			for (int i = 0; i < v->e_list->size(); i++)
			{
				if (is_holeEdge( (*(v->e_list))[i] ))
				{
					vertex *temp = vtx_init();
					temp->xyz.x = 0;
					temp->xyz.y = 0;
					temp->xyz.z = 0;
					temp->xyz.add ( (*(v->e_list))[i]->v1->xyz );
					temp->xyz.add ( (*(v->e_list))[i]->v2->xyz );
					temp->xyz.div(2.0f);
					v->v_new->xyz.add(temp->xyz);
					count++;
					delete temp;
				}
			}
			v->v_new->xyz.add(v->xyz);
			count++;
			v->v_new->xyz.div (count);
		}
		else
		{
			v->v_new = vtx_init();
			v->v_new->xyz.x = 0.0f;
			v->v_new->xyz.y = 0.0f;
			v->v_new->xyz.z = 0.0f;

			vertex *avg_face = vtx_init();
			avg_face->xyz.x = 0;
			avg_face->xyz.y = 0;
			avg_face->xyz.z = 0;
			for (int i = 0; i < v->f_list->size(); i++)
			{
				avg_face->xyz.add ( face_point ((*(v->f_list))[i])->xyz );
			}
			avg_face->xyz.div (v->f_list->size());
			vertex *avg_mid_points = vtx_init();
			avg_mid_points->xyz.x = 0;
			avg_mid_points->xyz.y = 0;
			avg_mid_points->xyz.z = 0;
			for (int i = 0; i < v->e_list->size(); i++)
			{
					vertex *temp = vtx_init();
					temp->xyz.x = 0;
					temp->xyz.y = 0;
					temp->xyz.z = 0;
					temp->xyz.add ( (*(v->e_list))[i]->v1->xyz );
					temp->xyz.add ( (*(v->e_list))[i]->v2->xyz );
					temp->xyz.div(2.0f);
					avg_mid_points->xyz.add(temp->xyz);
					delete temp;
			}		

			avg_mid_points->xyz.div (v->e_list->size());
			avg_mid_points->xyz.mul (2.0f);

			v->v_new->xyz.add(v->xyz);
			v->v_new->xyz.mul(v->f_list->size() - 3);
			v->v_new->xyz.add(avg_face->xyz);
			v->v_new->xyz.add(avg_mid_points->xyz);
			v->v_new->xyz.div(v->f_list->size());
			delete avg_face;
			delete avg_mid_points;
		}
	}
	//printf ("Vertex point (%3.3f, %3.3f, %3.3f)\n", v->v_new->xyz.x, v->v_new->xyz.y, v->v_new->xyz.z);
	return v->v_new; // delete this line after you fill in the blank.
}

object* catmull_clark(object* obj)
{
	object* newobj = obj_init();
	newobj->vertices_per_face = 4;

	/* fill in the blank */
	for (int i = 0; i < obj->f_list->size(); i++)
	{
		face *face_p = (*(obj->f_list))[i];
		if (obj->vertices_per_face == 3)
		{
			//printf ("Face idx %d: Triangular.\n", i);
			vertex *a = (*face_p->v_list)[0];
			vertex *b = (*face_p->v_list)[1];
			vertex *c = (*face_p->v_list)[2];

			edge *ab = find_edge (obj, a, b);
			edge *bc = find_edge (obj, b, c);
			edge *ca = find_edge (obj, c, a);

			face* abc = face_p;

			vertex *temp = vertex_point(a);
			vertex *vertex_point_a = find_if_not_add_vertex (newobj, temp->xyz);
			temp = vertex_point(b);
			vertex *vertex_point_b = find_if_not_add_vertex (newobj, temp->xyz);
			temp = vertex_point(c);
			vertex *vertex_point_c = find_if_not_add_vertex (newobj, temp->xyz);
			//printf ("Vertex points added.\n");
			temp = edge_point(ab);
			vertex *edge_point_ab = find_if_not_add_vertex (newobj, temp->xyz);
			temp = edge_point(bc);
			vertex *edge_point_bc = find_if_not_add_vertex (newobj, temp->xyz);
			temp = edge_point(ca);
			vertex *edge_point_ca = find_if_not_add_vertex (newobj, temp->xyz);
			//printf ("Edge points added.\n");
			temp = face_point(abc);
			vertex *face_point_abc = find_if_not_add_vertex (newobj, temp->xyz);
			//printf ("New vertice added.\n");

			add_face (newobj, {vertex_point_a->idx, edge_point_ab->idx, face_point_abc->idx, edge_point_ca->idx});
			add_face (newobj, {vertex_point_b->idx, edge_point_bc->idx, face_point_abc->idx, edge_point_ab->idx});
			add_face (newobj, {vertex_point_c->idx, edge_point_ca->idx, face_point_abc->idx, edge_point_bc->idx});

			//printf ("Face idx %d divided.\n", i);
		}
		else
		{
			//printf ("Face idx %d: Quad.\n", i);
			vertex *a = (*face_p->v_list)[0];
			vertex *b = (*face_p->v_list)[1];
			vertex *c = (*face_p->v_list)[2];
			vertex *d = (*face_p->v_list)[3];

			edge *ab = find_edge (obj, a, b);
			edge *bc = find_edge (obj, b, c);
			edge *cd = find_edge (obj, c, d);
			edge *da = find_edge (obj, d, a);

			face* abcd = face_p;

			vertex *temp = vertex_point(a);
			vertex *vertex_point_a = find_if_not_add_vertex (newobj, temp->xyz);
			temp = vertex_point(b);
			vertex *vertex_point_b = find_if_not_add_vertex (newobj, temp->xyz);
			temp = vertex_point(c);
			vertex *vertex_point_c = find_if_not_add_vertex (newobj, temp->xyz);
			temp = vertex_point(d);
			vertex *vertex_point_d = find_if_not_add_vertex (newobj, temp->xyz);
			//printf ("Vertex points added.\n");
			temp = edge_point(ab);
			vertex *edge_point_ab = find_if_not_add_vertex (newobj, temp->xyz);
			temp = edge_point(bc);
			vertex *edge_point_bc = find_if_not_add_vertex (newobj, temp->xyz);
			temp = edge_point(cd);
			vertex *edge_point_cd = find_if_not_add_vertex (newobj, temp->xyz);
			temp = edge_point(da);
			vertex *edge_point_da = find_if_not_add_vertex (newobj, temp->xyz);
			//printf ("Edge points added.\n");
			temp = face_point(abcd);
			vertex *face_point_abcd = find_if_not_add_vertex (newobj, temp->xyz);
			//printf ("New vertice added.\n");

			add_face (newobj, {vertex_point_a->idx, edge_point_ab->idx, face_point_abcd->idx, edge_point_da->idx});
			add_face (newobj, {vertex_point_b->idx, edge_point_bc->idx, face_point_abcd->idx, edge_point_ab->idx});
			add_face (newobj, {vertex_point_c->idx, edge_point_cd->idx, face_point_abcd->idx, edge_point_bc->idx});
			add_face (newobj, {vertex_point_d->idx, edge_point_da->idx, face_point_abcd->idx, edge_point_cd->idx});

			//printf ("Face idx %d divided.\n", i);
		}
	}
	//printf ("Faces divided.\n");


	setNorm(newobj);
	//printf ("Norm set.\n");

	aggregate_vertices(newobj);
	//printf ("Vertices aggregated.\n");

	delete obj;

	return newobj;
}
