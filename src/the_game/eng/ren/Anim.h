#ifndef ANIM_H
#define ANIM_H

#include <cstdint>
#include <cstring>

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace R {
	enum AnimBoneFlags { AnimHasTranslate = 1 };

	struct AnimBone {
		char		name[64];
		char		parent_name[64];
		int			id;
		int			offset;
		uint32_t	flags;
		glm::vec3	cur_pos;
		glm::quat	cur_rot;
	};

	struct Bone;

	struct AnimSequence {
		int			counter;
		char		name[64];
		int			fps;
		int			len;
		int			frame_size;
		float		frame_dur;
		float		anim_dur;
		float		*frames;
		AnimBone	*bones;
		size_t		num_bones;

		AnimSequence() : counter(0), fps(0), len(0), frame_size(0), frame_dur(0),
			anim_dur(0), frames(nullptr), bones(nullptr), num_bones(0) {
			memset(name, 0, sizeof(name));
		}

		std::vector<AnimBone *> LinkBones(std::vector<Bone> &bones);
		void Update(float delta, float *time);
		void InterpolateFrames(int fr_0, int fr_1, float t);
	};

	AnimSequence *GetAnimSequence(const struct AnimSequenceRef &ref);
	void ReleaseAnimSequence(AnimSequence &a);
	void ReleaseAnimSequence(AnimSequenceRef &ref);

	struct AnimSequenceRef {
		int index;

		AnimSequenceRef() : index(-1) {}
		~AnimSequenceRef() {
			this->Release();
		}

		AnimSequenceRef(const AnimSequenceRef &ref) {
			index = ref.index;
			if (index != -1) {
				GetAnimSequence(ref)->counter++;
			}
		}
		AnimSequenceRef(AnimSequenceRef &&ref) {
			index = ref.index;
			ref.index = -1;
		}
		AnimSequenceRef& operator=(const AnimSequenceRef &ref) {
			this->Release();
			index = ref.index;
			if (index != -1) {
				GetAnimSequence(ref)->counter++;
			}
			return *this;
		}
		AnimSequenceRef& operator=(AnimSequenceRef &&ref) {
			this->Release();
			index = ref.index;
			ref.index = -1;
			return *this;
		}

        AnimSequence *operator->() const {
            return R::GetAnimSequence(*this);
        }

		void Release() {
			ReleaseAnimSequence(*this);
		}
	};

	AnimSequenceRef LoadAnimSequence(const char *name, void *data);

	void InitAnimSequence(AnimSequence &anim, void *data);

	struct AnimLink {
		float					anim_time;
		AnimSequenceRef			anim;
		std::vector<AnimBone *> anim_bones;
	};

	struct Bone {
		char		name[64];
		int			id;
		int			parent_id;
		int			dirty;
		glm::mat4	cur_matrix;
		glm::mat4	cur_comb_matrix;
		glm::mat4	bind_matrix;
		glm::mat4	inv_bind_matrix;
		glm::vec3	head_pos;
	};

    struct BoneGroup {
        std::vector<int> strip_ids;
        std::vector<int> bone_ids;
    };

	struct Skeleton {
		std::vector<Bone>       bones;
		std::vector<AnimLink>   anims;
        std::vector<glm::mat4>  matr_palette;
        std::vector<BoneGroup>  bone_groups;

        std::vector<Bone>::iterator bone(const char *name) {
            auto b = bones.begin();
            for (; b != bones.end(); ++b) {
                if (strcmp(b->name, name) == 0) {
                    break;
                }
            }
            return b;
        }

        int bone_index(const char *name) {
            auto b = bone(name);
            if (b == bones.end()) {
                return -1;
            } else {
                return int(b - bones.begin());
            }
        }

        glm::vec3 bone_pos(const char *name);
		glm::vec3 bone_pos(int i);

        void bone_matrix(const char *name, glm::mat4 &mat);
        void bone_matrix(int i, glm::mat4 &mat);

        int AddAnimSequence(const char *name, void *data);

        void MarkChildren();
        void ApplyAnim(int id);
        void ApplyAnim(int anim_id1, int anim_id2, float t);
        void UpdateAnim(int anim_id, float delta, float *t);
        void UpdateBones();
    };
}

#endif // ANIM_H