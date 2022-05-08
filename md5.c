#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "math.h"
#include "md5.h"

#ifndef FREE
#define FREE(p) { if(p) { free(p); p=NULL; } }
#endif

typedef struct
{
	char name[64];
	int32_t parent;
	int32_t flags;
	int32_t startIndex;
} Joint_Info_t;

typedef struct
{
	float pos[3];
	float orient[4];
} BaseFrame_Joint_t;

void Quat_computeW(float *q)
{
	float t=1.0f-(q[0]*q[0])-(q[1]*q[1])-(q[2]*q[2]);

	if(t<0.0f)
		q[3]=0.0f;
	else
		q[3]=-sqrtf(t);
}

void Quat_multVec(const float q[4], const float v[3], float *out)
{
	out[0]= (q[3]*v[0])+(q[1]*v[2])-(q[2]*v[1]);
	out[1]= (q[3]*v[1])+(q[2]*v[0])-(q[0]*v[2]);
	out[2]= (q[3]*v[2])+(q[0]*v[1])-(q[1]*v[0]);
	out[3]=-(q[0]*v[0])-(q[1]*v[1])-(q[2]*v[2]);
}

void Quat_rotatePoint(const float q[4], const float in[3], float *out)
{
	float tmp[4], inv[4]={ q[0], q[1], q[2], q[3] };
	vec4 final;

	QuatInverse(inv);

	Quat_multVec(q, in, tmp);
	QuatMultiply(tmp, inv, final);

	out[0]=final[0];
	out[1]=final[1];
	out[2]=final[2];
}

int32_t LoadMD5(MD5_Model_t *mdl, const char *filename)
{
	FILE *fp;
	char buff[512];
	int32_t version;
	int32_t curr_mesh=0;
	int32_t i, j;

	if(!(fp=fopen(filename, "rb")))
		return 0;

	while(!feof(fp))
	{
		// Read line
		fgets(buff, sizeof(buff), fp);

		if(sscanf(buff, " MD5Version %d", &version)==1)
		{
			if(version!=10)
			{
				// Wrong version
				fprintf(stderr, "Wrong model version\n");
				fclose(fp);
				return 0;
			}
		}
		else if(sscanf(buff, " numJoints %d", &mdl->num_joints)==1)
		{
			// Allocate memory for base skeleton joints
			if(mdl->num_joints>0)
				mdl->baseSkel=(MD5_Joint_t *)malloc(sizeof(MD5_Joint_t)*mdl->num_joints);
		}
		else if(sscanf(buff, " numMeshes %d", &mdl->num_meshes)==1)
		{
			// Allocate memory for meshes
			if(mdl->num_meshes>0)
				mdl->meshes=(MD5_Mesh_t *)malloc(sizeof(MD5_Mesh_t)*mdl->num_meshes);
		}
		else if(strncmp(buff, "joints {", 8)==0)
		{
			// Read each joint
			for(i=0;i<mdl->num_joints;i++)
			{
				MD5_Joint_t *joint=&mdl->baseSkel[i];

				// Read line
				fgets(buff, sizeof(buff), fp);

				if(sscanf(buff, "%s %d ( %f %f %f ) ( %f %f %f )", joint->name, &joint->parent, &joint->pos[0], &joint->pos[1], &joint->pos[2], &joint->orient[0], &joint->orient[1], &joint->orient[2])==8)
					Quat_computeW(joint->orient); // Compute the w component
			}
		}
		else if(strncmp(buff, "mesh {", 6)==0)
		{
			MD5_Mesh_t *mesh=&mdl->meshes[curr_mesh];
			int32_t vert_index=0;
			int32_t tri_index=0;
			int32_t weight_index=0;
			float fdata[4];
			int32_t idata[3];

			while((buff[0]!='}')&&!feof(fp))
			{
				// Read line
				fgets(buff, sizeof(buff), fp);

				if(strstr(buff, "shader "))
				{
					int32_t quote=0;

					// Copy the shader name whithout the quote marks
					for(i=0, j=0;i<sizeof(buff)&&(quote<2);i++)
					{
						if(buff[i]=='\"')
							quote++;

						if((quote==1)&&(buff[i]!='\"'))
						{
							mesh->shader[j]=buff[i];
							j++;
						}
					}
				}
				else if(sscanf(buff, " numverts %d", &mesh->num_verts)==1)
				{
					// Allocate memory for vertices
					if(mesh->num_verts>0)
						mesh->vertices=(MD5_Vertex_t *)malloc(sizeof(MD5_Vertex_t)*mesh->num_verts);
				}
				else if(sscanf(buff, " numtris %d", &mesh->num_tris)==1)
				{
					// Allocate memory for triangles
					if(mesh->num_tris>0)
						mesh->triangles=(uint32_t *)malloc(sizeof(uint32_t)*mesh->num_tris*3);
				}
				else if(sscanf(buff, " numweights %d", &mesh->num_weights)==1)
				{
					// Allocate memory for vertex weights
					if(mesh->num_weights>0)
						mesh->weights=(MD5_Weight_t *)malloc(sizeof(MD5_Weight_t)*mesh->num_weights);
				}
				else if(sscanf(buff, " vert %d ( %f %f ) %d %d", &vert_index, &fdata[0], &fdata[1], &idata[0], &idata[1])==5)
				{
					// Copy vertex data
					mesh->vertices[vert_index].st[0]=fdata[0];
					mesh->vertices[vert_index].st[1]=fdata[1];
					mesh->vertices[vert_index].start=idata[0];
					mesh->vertices[vert_index].count=idata[1];
				}
				else if(sscanf(buff, " tri %d %d %d %d", &tri_index, &idata[0], &idata[1], &idata[2])==4)
				{
					// Copy triangle indices
					mesh->triangles[3*tri_index+0]=idata[2];
					mesh->triangles[3*tri_index+1]=idata[1];
					mesh->triangles[3*tri_index+2]=idata[0];
				}
				else if(sscanf(buff, " weight %d %d %f ( %f %f %f )", &weight_index, &idata[0], &fdata[3], &fdata[0], &fdata[1], &fdata[2])==6)
				{
					// Copy weight data
					mesh->weights[weight_index].joint=idata[0];
					mesh->weights[weight_index].bias=fdata[3];
					mesh->weights[weight_index].pad[0]=0.0f;
					mesh->weights[weight_index].pad[1]=0.0f;
					mesh->weights[weight_index].pos[0]=fdata[0];
					mesh->weights[weight_index].pos[1]=fdata[1];
					mesh->weights[weight_index].pos[2]=fdata[2];
					mesh->weights[weight_index].pos[3]=1.0f;
				}
			}

			// Allocate temp memory for the bind pose vertex data
			float *temppos=(float *)malloc(sizeof(float)*mesh->num_verts*3);
			float *temptang=(float *)malloc(sizeof(float)*mesh->num_verts*3);
			float *tempbinorm=(float *)malloc(sizeof(float)*mesh->num_verts*3);
			float *tempnorm=(float *)malloc(sizeof(float)*mesh->num_verts*3);

			// Compute the vertex positions for the model in bind pose
			for(i=0;i<mesh->num_verts;i++)
			{
				// Clear temp vertex data
				temppos[3*i+0]=0.0f;		temppos[3*i+1]=0.0f;		temppos[3*i+2]=0.0f;
				temptang[3*i+0]=0.0f;		temptang[3*i+1]=0.0f;		temptang[3*i+2]=0.0f;
				tempbinorm[3*i+0]=0.0f;		tempbinorm[3*i+1]=0.0f;		tempbinorm[3*i+2]=0.0f;
				tempnorm[3*i+0]=0.0f;		tempnorm[3*i+1]=0.0f;		tempnorm[3*i+2]=0.0f;

				// Calculate final vertex to draw with weights
				for(j=0;j<mesh->vertices[i].count;j++)
				{
					MD5_Weight_t *weight=&mesh->weights[mesh->vertices[i].start+j];
					const MD5_Joint_t *joint=&mdl->baseSkel[weight->joint];
					float temp[3];

					// Rotate by joint
					Quat_rotatePoint(joint->orient, weight->pos, temp);

					// Add position and weight the vertex by the bias
					temppos[3*i+0]+=(joint->pos[0]+temp[0])*weight->bias;
					temppos[3*i+1]+=(joint->pos[1]+temp[1])*weight->bias;
					temppos[3*i+2]+=(joint->pos[2]+temp[2])*weight->bias;
				}
			}

			// Compute tangent space (tangent, binormal, and normal) vectors for the bind pose
			float v0[3], v1[3], uv0[2], uv1[2];
			float s[3], t[3], n[3], r;

			for(i=0;i<mesh->num_tris;i++)
			{
				uint32_t i1=mesh->triangles[3*i+0];
				uint32_t i2=mesh->triangles[3*i+1];
				uint32_t i3=mesh->triangles[3*i+2];

				v0[0]=temppos[3*i2+0]-temppos[3*i1+0];
				v0[1]=temppos[3*i2+1]-temppos[3*i1+1];
				v0[2]=temppos[3*i2+2]-temppos[3*i1+2];

				uv0[0]=mesh->vertices[i2].st[0]-mesh->vertices[i1].st[0];
				uv0[1]=mesh->vertices[i2].st[1]-mesh->vertices[i1].st[1];

				v1[0]=temppos[3*i3+0]-temppos[3*i1+0];
				v1[1]=temppos[3*i3+1]-temppos[3*i1+1];
				v1[2]=temppos[3*i3+2]-temppos[3*i1+2];

				uv1[0]=mesh->vertices[i3].st[0]-mesh->vertices[i1].st[0];
				uv1[1]=mesh->vertices[i3].st[1]-mesh->vertices[i1].st[1];

				r=1.0f/(uv0[0]*uv1[1]-uv1[0]*uv0[1]);

				s[0]=(uv1[1]*v0[0]-uv0[1]*v1[0])*r;
				s[1]=(uv1[1]*v0[1]-uv0[1]*v1[1])*r;
				s[2]=(uv1[1]*v0[2]-uv0[1]*v1[2])*r;
				Vec3_Normalize(s);

				t[0]=(uv0[0]*v1[0]-uv1[0]*v0[0])*r;
				t[1]=(uv0[0]*v1[1]-uv1[0]*v0[1])*r;
				t[2]=(uv0[0]*v1[2]-uv1[0]*v0[2])*r;
				Vec3_Normalize(t);

				Cross(v0, v1, n);
				Vec3_Normalize(n);

				Vec3_Addv(&temptang[3*i1], s);
				Vec3_Addv(&temptang[3*i2], s);
				Vec3_Addv(&temptang[3*i3], s);

				Vec3_Addv(&tempbinorm[3*i1], t);
				Vec3_Addv(&tempbinorm[3*i2], t);
				Vec3_Addv(&tempbinorm[3*i3], t);

				Vec3_Addv(&tempnorm[3*i1], n);
				Vec3_Addv(&tempnorm[3*i2], n);
				Vec3_Addv(&tempnorm[3*i3], n);
			}

			// Clear weighted tangent space vectors
			for(i=0;i<mesh->num_weights;i++)
			{
				Vec4_Sets(mesh->weights[i].tangent, 0.0f);
				Vec4_Sets(mesh->weights[i].binormal, 0.0f);
				Vec4_Sets(mesh->weights[i].normal, 0.0f);
			}

			// Rotate the tangent space vectors into joint space by using the inverse joint orientation
			for(i=0;i<mesh->num_verts;i++)
			{
				for(j=0;j<mesh->vertices[i].count;j++)
				{
					MD5_Weight_t *weight=&mesh->weights[mesh->vertices[i].start+j];
					const MD5_Joint_t *joint=&mdl->baseSkel[weight->joint];
					float temp[3];
					vec4 inv={ joint->orient[0], joint->orient[1], joint->orient[2], joint->orient[3] };

					QuatInverse(inv);

					Quat_rotatePoint(inv, &temptang[3*i], temp);
					Vec3_Addv(weight->tangent, temp);
					weight->tangent[3]=0.0f;

					Quat_rotatePoint(inv, &tempbinorm[3*i], temp);
					Vec3_Addv(weight->binormal, temp);
					weight->binormal[3]=0.0f;

					Quat_rotatePoint(inv, &tempnorm[3*i], temp);
					Vec3_Addv(weight->normal, temp);
					weight->normal[3]=0.0f;
				}
			}

			for(i=0;i<mesh->num_weights;i++)
			{
				Vec4_Normalize(mesh->weights[i].tangent);
				Vec4_Normalize(mesh->weights[i].binormal);
				Vec4_Normalize(mesh->weights[i].normal);
			}

			FREE(temppos);
			FREE(temptang);
			FREE(tempbinorm);
			FREE(tempnorm);

//			mesh->vertexArray=(float *)malloc(sizeof(float)*mesh->num_verts*20);

			curr_mesh++;
		}
	}

	fclose(fp);

	return 1;
}

// Free memory allocated for the model
void FreeMD5(MD5_Model_t *Model)
{
	int32_t i;

	if(Model->baseSkel)
		FREE(Model->baseSkel);

	if(Model->meshes)
	{
		/* Free mesh data */
		for(i=0;i<Model->num_meshes;i++)
		{
			if(Model->meshes[i].vertices)
				FREE(Model->meshes[i].vertices);

			if(Model->meshes[i].triangles)
				FREE(Model->meshes[i].triangles);

			if(Model->meshes[i].weights)
				FREE(Model->meshes[i].weights);

			//if(Model->meshes[i].vertexArray)
			//	FREE(Model->meshes[i].vertexArray);
		}

		FREE(Model->meshes);
	}
}

void PrepareMesh(MD5_Mesh_t *mesh, const MD5_Joint_t *skeleton, float *vertexArray)
{
	int32_t i, j;

	for(i=0;i<mesh->num_verts;i++)
	{
		float finalVertex[3]={ 0.0f, 0.0f, 0.0f };
		float finalNormal[3]={ 0.0f, 0.0f, 0.0f };
		float finalTangent[3]={ 0.0f, 0.0f, 0.0f };
		float finalBinormal[3]={ 0.0f, 0.0f, 0.0f };

		/* Calculate final vertex to draw with weights */
		for(j=0;j<mesh->vertices[i].count;j++)
		{
			const MD5_Weight_t *weight=&mesh->weights[mesh->vertices[i].start+j];
			const MD5_Joint_t *joint=&skeleton[weight->joint];
			float temp[3];

			Quat_rotatePoint(joint->orient, weight->pos, temp);
			finalVertex[0]+=(joint->pos[0]+temp[0])*weight->bias;
			finalVertex[1]+=(joint->pos[1]+temp[1])*weight->bias;
			finalVertex[2]+=(joint->pos[2]+temp[2])*weight->bias;

			Quat_rotatePoint(joint->orient, weight->tangent, temp);
			finalTangent[0]+=temp[0]*weight->bias;
			finalTangent[1]+=temp[1]*weight->bias;
			finalTangent[2]+=temp[2]*weight->bias;

			Quat_rotatePoint(joint->orient, weight->binormal, temp);
			finalBinormal[0]+=temp[0]*weight->bias;
			finalBinormal[1]+=temp[1]*weight->bias;
			finalBinormal[2]+=temp[2]*weight->bias;

			Quat_rotatePoint(joint->orient, weight->normal, temp);
			finalNormal[0]+=temp[0]*weight->bias;
			finalNormal[1]+=temp[1]*weight->bias;
			finalNormal[2]+=temp[2]*weight->bias;
		}

		vertexArray[20*i+0]=finalVertex[0];
		vertexArray[20*i+1]=finalVertex[1];
		vertexArray[20*i+2]=finalVertex[2];
		vertexArray[20*i+3]=1.0f;
		vertexArray[20*i+4]=mesh->vertices[i].st[0];
		vertexArray[20*i+5]=1.0f-mesh->vertices[i].st[1];
		vertexArray[20*i+6]=0.0f;
		vertexArray[20*i+7]=0.0f;
		vertexArray[20*i+8]=finalTangent[0];
		vertexArray[20*i+9]=finalTangent[1];
		vertexArray[20*i+10]=finalTangent[2];
		vertexArray[20*i+11]=1.0f;
		vertexArray[20*i+12]=finalBinormal[0];
		vertexArray[20*i+13]=finalBinormal[1];
		vertexArray[20*i+14]=finalBinormal[2];
		vertexArray[20*i+15]=1.0f;
		vertexArray[20*i+16]=finalNormal[0];
		vertexArray[20*i+17]=finalNormal[1];
		vertexArray[20*i+18]=finalNormal[2];
		vertexArray[20*i+19]=1.0f;
	}
}

// Load an MD5 animation from file.
int32_t LoadAnim(MD5_Anim_t *anim, const char *filename)
{
	FILE *fp=NULL;
	char buff[512];
	Joint_Info_t *jointInfos=NULL;
	BaseFrame_Joint_t *baseFrame=NULL;
	float *animFrameData=NULL;
	int32_t version;
	int32_t numAnimatedComponents;
	int32_t frame_index;
	int32_t i;

	if(!(fp=fopen(filename, "rb")))
		return 0;

	while(!feof(fp))
	{
		// Read line
		fgets(buff, sizeof(buff), fp);

		if(sscanf(buff, " MD5Version %d", &version)==1)
		{
			if(version!=10)
			{
				// Wrong version
				fprintf(stderr, "Error: bad animation version\n");
				fclose(fp);
				return 0;
			}
		}
		else if(sscanf(buff, " numFrames %d", &anim->num_frames)==1)
		{
			// Allocate memory for skeleton frames and bounding boxes
			if(anim->num_frames>0)
			{
				anim->skelFrames=(MD5_Joint_t **)malloc(sizeof(MD5_Joint_t *)*anim->num_frames);
				anim->bboxes=(MD5_BBox_t *)malloc(sizeof(MD5_BBox_t)*anim->num_frames);
			}
		}
		else if(sscanf(buff, " numJoints %d", &anim->num_joints)==1)
		{
			if(anim->num_joints>0)
			{
				// Allocate memory for joints of each frame
				for(i=0; i<anim->num_frames; ++i)
					anim->skelFrames[i]=(MD5_Joint_t *)malloc(sizeof(MD5_Joint_t)*anim->num_joints);

				// Allocate temporary memory for building skeleton frames
				jointInfos=(Joint_Info_t *)malloc(sizeof(Joint_Info_t)*anim->num_joints);
				baseFrame=(BaseFrame_Joint_t *)malloc(sizeof(BaseFrame_Joint_t)*anim->num_joints);
			}
		}
		else if(sscanf(buff, " frameRate %d", &anim->frameRate)==1) { }
		else if(sscanf(buff, " numAnimatedComponents %d", &numAnimatedComponents)==1)
		{
			if(numAnimatedComponents>0)
			{
				// Allocate memory for animation frame
				animFrameData=(float *)malloc(sizeof(float)*numAnimatedComponents);
			}
		}
		else if(strncmp(buff, "hierarchy {", 11)==0)
		{
			for(i=0; i<anim->num_joints; ++i)
			{
				// Read line
				fgets(buff, sizeof(buff), fp);

				// Read joint info
				sscanf(buff, " %s %d %d %d", jointInfos[i].name, &jointInfos[i].parent, &jointInfos[i].flags, &jointInfos[i].startIndex);
			}
		}
		else if(strncmp(buff, "bounds {", 8)==0)
		{
			for(i=0;i<anim->num_frames;i++)
			{
				// Read line
				fgets(buff, sizeof(buff), fp);

				// Read bounding box
				sscanf(buff, " ( %f %f %f ) ( %f %f %f )", &anim->bboxes[i].min[0], &anim->bboxes[i].min[1], &anim->bboxes[i].min[2], &anim->bboxes[i].max[0], &anim->bboxes[i].max[1], &anim->bboxes[i].max[2]);
			}
		}
		else if(strncmp(buff, "baseframe {", 10)==0)
		{
			for(i=0; i<anim->num_joints; ++i)
			{
				// Read line
				fgets(buff, sizeof(buff), fp);

				// Read base frame joint
				if(sscanf(buff, " ( %f %f %f ) ( %f %f %f )", &baseFrame[i].pos[0], &baseFrame[i].pos[1], &baseFrame[i].pos[2], &baseFrame[i].orient[0], &baseFrame[i].orient[1], &baseFrame[i].orient[2])==6)
					Quat_computeW(baseFrame[i].orient); // Compute the w component
			}
		}
		else if(sscanf(buff, " frame %d", &frame_index)==1)
		{
			// Read frame data
			for(i=0; i<numAnimatedComponents; ++i)
				fscanf(fp, "%f", &animFrameData[i]);

			// Build frame skeleton from the collected data
			for(i=0;i<anim->num_joints;++i)
			{
				BaseFrame_Joint_t *baseJoint=&baseFrame[i];
				float animatedPos[3], animatedOrient[4];
				int32_t j=0;

				memcpy(animatedPos, baseJoint->pos, sizeof(float)*3);
				memcpy(animatedOrient, baseJoint->orient, sizeof(float)*4);

				if(jointInfos[i].flags&1) // Pos X
					animatedPos[0]=animFrameData[jointInfos[i].startIndex+j++];

				if(jointInfos[i].flags&2) // Pos Y
					animatedPos[1]=animFrameData[jointInfos[i].startIndex+j++];

				if(jointInfos[i].flags&4) // Pos Z
					animatedPos[2]=animFrameData[jointInfos[i].startIndex+j++];

				if(jointInfos[i].flags&8) // Quat X
					animatedOrient[0]=animFrameData[jointInfos[i].startIndex+j++];

				if(jointInfos[i].flags&16) // Quat Y
					animatedOrient[1]=animFrameData[jointInfos[i].startIndex+j++];

				if(jointInfos[i].flags&32) // Quat Z
					animatedOrient[2]=animFrameData[jointInfos[i].startIndex+j++];

				// Compute the w component
				Quat_computeW(animatedOrient);

				MD5_Joint_t *thisJoint=&anim->skelFrames[frame_index][i];

				int32_t parent=jointInfos[i].parent;

				thisJoint->parent=parent;

				strcpy(thisJoint->name, jointInfos[i].name);

				// Parent joint is -1
				if(thisJoint->parent<0)
				{
					memcpy(thisJoint->pos, animatedPos, sizeof(float)*3);
					memcpy(thisJoint->orient, animatedOrient, sizeof(float)*4);
				}
				else
				{
					MD5_Joint_t *parentJoint=&anim->skelFrames[frame_index][parent];

					// Rotated position
					Quat_rotatePoint(parentJoint->orient, animatedPos, thisJoint->pos);

					// Add positions
					thisJoint->pos[0]+=parentJoint->pos[0];
					thisJoint->pos[1]+=parentJoint->pos[1];
					thisJoint->pos[2]+=parentJoint->pos[2];

					// Concatenate rotations
					QuatMultiply(parentJoint->orient, animatedOrient, thisJoint->orient);
					Vec4_Normalize(thisJoint->orient);
				}
			}
		}
	}

	fclose(fp);

	/* Free temporary data allocated */
	if(animFrameData)
		FREE(animFrameData);

	if(baseFrame)
		FREE(baseFrame);

	if(jointInfos)
		FREE(jointInfos);

	return 1;
}

// Free memory allocated for animation.
void FreeAnim(MD5_Anim_t *anim)
{
	int32_t i;

	if(anim->skelFrames)
	{
		for(i=0;i<anim->num_frames;++i)
		{
			if(anim->skelFrames[i])
				FREE(anim->skelFrames[i]);
		}

		FREE(anim->skelFrames);
	}

	if(anim->bboxes)
		FREE(anim->bboxes);
}

void InterpolateSkeletons(const MD5_Anim_t *Anim, const MD5_Joint_t *skelA, const MD5_Joint_t *skelB, float interp, MD5_Joint_t *out)
{
	for(int32_t i=0;i<Anim->num_joints;i++)
	{
		// Copy parent index
		out[i].parent=skelA[i].parent;

		// Liear interpolate position
		Vec3_Lerp(skelA[i].pos, skelB[i].pos, interp, out[i].pos);

		// Spherical interpolate rotation
		QuatSlerp(skelA[i].orient, skelB[i].orient, out[i].orient, interp);
	}
}
