#include "Anim.h"

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SparseArray.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace R {
	SparseArray<AnimSequence> anim_sequences(128);
}

R::AnimSequenceRef R::LoadAnimSequence(const char *name, void *data) {
	auto it = anim_sequences.begin();
	for (; it != anim_sequences.end(); it++) {
		if (strcmp(name, it->name) == 0) {
			break;
		}
	}
	if (it != anim_sequences.end()) {
		it->counter++;

		AnimSequenceRef ref;
		ref.index = (int) it.index();

		return ref;
	} else {
		AnimSequence a;
		a.counter = 1;
		strcpy(a.name, name);

		char anim_type_str[12];
		memcpy(anim_type_str, data, 12);

		if (strcmp(anim_type_str, "ANIM_SEQUEN\0") == 0) {
			InitAnimSequence(a, data);
		}

		AnimSequenceRef ref;
		ref.index = (int)anim_sequences.Add(a);

		return ref;
	}
}

R::AnimSequence *R::GetAnimSequence(const struct AnimSequenceRef &ref) {
    assert(ref.index != -1);
    return anim_sequences.Get((size_t)ref.index);
}

void R::ReleaseAnimSequence(AnimSequence &a) {
	delete[] a.bones;
	a.bones = nullptr;
	a.num_bones = 0;
	delete[] a.frames;
	a.frames = nullptr;

	a.name[0] = '\0';
}

void R::ReleaseAnimSequence(AnimSequenceRef &ref) {
	if (ref.index == -1) {
		return;
	}
	AnimSequence *a = anim_sequences.Get((size_t)ref.index);
	if (!--a->counter) {
		ReleaseAnimSequence(*a);
		anim_sequences.Remove((size_t)ref.index);
	}
	ref.index = -1;
}

#define READ_ADVANCE(dest, p, size) memcpy(dest, p, size); p += size;

void R::InitAnimSequence(AnimSequence &a, void *data) {
	char *p = (char *)data;
	
	char str[64];
	READ_ADVANCE(str, p, 12);
	assert(strcmp(str, "ANIM_SEQUEN\0") == 0);

	enum {	SKELETON_CHUNK,
			ANIM_INFO_CHUNK,
			FRAMES_CHUNK };

	struct ChunkPos {
		int offset;
		int length;
	};

	struct Header {
		int num_chunks;
		ChunkPos p[3];
	} file_header;

	READ_ADVANCE(&file_header, p, sizeof(file_header));
	
	a.num_bones = (size_t) file_header.p[SKELETON_CHUNK].length / (64 + 64 + 4);
	a.bones = new AnimBone[a.num_bones];
	int offset = 0;
	for (size_t i = 0; i < a.num_bones; i++) {
		a.bones[i].id = (int)i;
        a.bones[i].flags = 0;
		READ_ADVANCE(a.bones[i].name, p, 64);
		READ_ADVANCE(a.bones[i].parent_name, p, 64);
		int has_translate_anim = 0;
		READ_ADVANCE(&has_translate_anim, p, 4);
		if (has_translate_anim) a.bones[i].flags |= AnimHasTranslate;
		a.bones[i].offset = offset;
		if (has_translate_anim) {
			offset += 7;
		} else {
			offset += 4;
		}
	}
	a.frame_size = offset;
	READ_ADVANCE(a.name, p, 64);
	READ_ADVANCE(&a.fps, p, 4);
	READ_ADVANCE(&a.len, p, 4);

	a.frames = new float[file_header.p[FRAMES_CHUNK].length / 4];
	READ_ADVANCE(a.frames, p, (size_t) file_header.p[FRAMES_CHUNK].length);

	a.frame_dur = 1.0f / a.fps;
	a.anim_dur = a.len * a.frame_dur;
}

// AnimSequence

std::vector<R::AnimBone *> R::AnimSequence::LinkBones(std::vector<Bone> &_bones) {
	std::vector<AnimBone *> anim_bones;
	anim_bones.reserve(_bones.size());
	for (size_t i = 0; i < _bones.size(); i++) {
		bool added = false;
		for (size_t j = 0; j < this->num_bones; j++) {
			if (strcmp(_bones[i].name, this->bones[j].name) == 0) {
				if (_bones[i].parent_id != -1) {
					assert(strcmp(_bones[_bones[i].parent_id].name, this->bones[j].parent_name) == 0);
				}
				anim_bones.push_back(&this->bones[j]);
				added = true;
                break;
			}
		}
		if (!added) {
			anim_bones.push_back(nullptr);
		}
	}
	return anim_bones;
}

void R::AnimSequence::Update(float delta, float *time) {
	if (len < 2)return;
	*time += delta;

	while (*time > anim_dur)*time -= anim_dur;
	while (*time < 0.0f)*time += anim_dur;

	float frame = *time * (float)fps;
	int fr_0 = (int)glm::floor(frame);
	int fr_1 = (int)glm::ceil(frame);

	fr_0 = fr_0 % len;
	fr_1 = fr_1 % len;
	float t = glm::mod(*time, frame_dur) / frame_dur;
	InterpolateFrames(fr_0, fr_1, t);
}

void R::AnimSequence::InterpolateFrames(int fr_0, int fr_1, float t) {
	for (size_t i = 0; i < num_bones; i++) {
		int offset = bones[i].offset;
		if (bones[i].flags & AnimHasTranslate) {
			glm::vec3 p1 = glm::make_vec3(&frames[fr_0 * frame_size + offset]);
			glm::vec3 p2 = glm::make_vec3(&frames[fr_1 * frame_size + offset]);
			bones[i].cur_pos = glm::mix(p1, p2, t);
			offset += 3;
		}
		glm::quat q1 = glm::make_quat(&frames[fr_0 * frame_size + offset]);
		glm::quat q2 = glm::make_quat(&frames[fr_1 * frame_size + offset]);
		bones[i].cur_rot = glm::mix(q1, q2, t);
	}
}

// skeleton

glm::vec3 R::Skeleton::bone_pos(const char *name) {
    auto b = bone(name);
    glm::vec3 ret;
    const float *m = &(b->cur_comb_matrix)[0][0];
    /*ret[0] = -(m[0] * m[12] + m[1] * m[13] + m[2] * m[14]);
    ret[1] = -(m[4] * m[12] + m[5] * m[13] + m[6] * m[14]);
    ret[2] = -(m[8] * m[12] + m[9] * m[13] + m[10] * m[14]);*/

    ret[0] = m[12];
    ret[1] = m[13];
    ret[2] = m[14];

    return ret;
}

glm::vec3 R::Skeleton::bone_pos(int i) {
    auto b = &bones[i];
    glm::vec3 ret;
    const float *m = &(b->cur_comb_matrix)[0][0];
    /*ret[0] = -(m[0] * m[12] + m[1] * m[13] + m[2] * m[14]);
    ret[1] = -(m[4] * m[12] + m[5] * m[13] + m[6] * m[14]);
    ret[2] = -(m[8] * m[12] + m[9] * m[13] + m[10] * m[14]);*/

    ret[0] = m[12];
    ret[1] = m[13];
    ret[2] = m[14];

    return ret;
}

void R::Skeleton::bone_matrix(const char *name, glm::mat4 &mat) {
    auto b = bone(name);
    UpdateBones();
    assert(b != bones.end());
    mat = b->cur_comb_matrix;
}

void R::Skeleton::bone_matrix(int i, glm::mat4 &mat) {
    UpdateBones();
    mat = bones[i].cur_comb_matrix;
}

void R::Skeleton::UpdateBones() {
    for (size_t i = 0; i < bones.size(); i++) {
        if (bones[i].dirty) {
            if (bones[i].parent_id != -1) {
                bones[i].cur_comb_matrix = bones[bones[i].parent_id].cur_comb_matrix * bones[i].cur_matrix;
            } else {
                bones[i].cur_comb_matrix = bones[i].cur_matrix;
            }
            matr_palette[i] = bones[i].cur_comb_matrix * bones[i].inv_bind_matrix;
            bones[i].dirty = false;
        }
    }
}

int R::Skeleton::AddAnimSequence(const char *name, void *data) {
    anims.emplace_back();
    auto &a = anims.back();
    a.anim = R::LoadAnimSequence(name, data);
    a.anim_time = 0.0f;
    a.anim_bones = anims[anims.size() - 1].anim->LinkBones(bones);
    return int(anims.size() - 1);
}

void R::Skeleton::MarkChildren() {
    for (size_t i = 0; i < bones.size(); i++) {
        if (bones[i].parent_id != -1 && bones[bones[i].parent_id].dirty) {
            bones[i].dirty = true;
        }
    }
}

void R::Skeleton::ApplyAnim(int id) {
    for (size_t i = 0; i < bones.size(); i++) {
        if (anims[id].anim_bones[i]) {
            glm::mat4 m(1.0f);
            if (anims[id].anim_bones[i]->flags & AnimHasTranslate) {
                m = glm::translate(m, anims[id].anim_bones[i]->cur_pos);
            } else {
                m = glm::translate(m, bones[i].head_pos);
            }
            m *= glm::toMat4(anims[id].anim_bones[i]->cur_rot);
            bones[i].cur_matrix = m;
            bones[i].dirty = true;
        }
    }
    MarkChildren();
}

void R::Skeleton::ApplyAnim(int anim_id1, int anim_id2, float t) {
    for (size_t i = 0; i < bones.size(); i++) {
        if (anims[anim_id1].anim_bones[i] || anims[anim_id2].anim_bones[i]) {
            glm::mat4 m(1.0f);
            glm::vec3 pos;
            glm::quat orient;
            if (anims[anim_id1].anim_bones[i]) {
                if (anims[anim_id1].anim_bones[i]->flags & AnimHasTranslate) {
                    pos = anims[anim_id1].anim_bones[i]->cur_pos;
                } else {
                    pos = bones[i].head_pos;
                }
                orient = anims[anim_id1].anim_bones[i]->cur_rot;
            }
            if (anims[anim_id2].anim_bones[i]) {
                if (anims[anim_id2].anim_bones[i]->flags & AnimHasTranslate) {
                    pos = glm::mix(pos, anims[anim_id2].anim_bones[i]->cur_pos, t);
                }
                orient = glm::slerp(orient, anims[anim_id2].anim_bones[i]->cur_rot, t);
            }
            m = glm::translate(m, pos);
            m *= glm::toMat4(orient);
            bones[i].cur_matrix = m;
            bones[i].dirty = true;
        }
    }
    MarkChildren();
}

void R::Skeleton::UpdateAnim(int anim_id, float delta, float *t) {
    if (!t) {
        t = &anims[anim_id].anim_time;
    }
    anims[anim_id].anim->Update(delta, t);
}