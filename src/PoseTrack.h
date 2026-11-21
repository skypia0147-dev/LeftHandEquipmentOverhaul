#pragma once

// Read and write one bone of a generator output's pose track (hkbGeneratorOutput, CommonLib only
// forward-declares it). Layout as in Animation Injector's PoseTrackReader, cross-checked against
// Open Animation Replacer's Havok.h: a master header, one header per track, then the track data.
namespace LHEO::PoseTrack {

constexpr int kPoseTrack = 2;  // hkbGeneratorOutput::TRACK_POSE

struct Master {
    std::int32_t bytes, tracks;
    std::array<std::byte, 8> unused;
};
struct Header {
    std::int16_t capacity, count, offset, stride;
    float fraction;
    std::uint8_t flags, type;
    std::uint16_t padding;
};
static_assert(sizeof(Master) == 16 && sizeof(Header) == 16);

struct Output {
    Master* tracks;
    bool deleteTracks;
};
static_assert(sizeof(Output) == 16);

// One bone's hkQsTransform: translation 0..2, rotation 4..7 (x, y, z, w), scale 8..10.
using Qs = std::array<float, 12>;

// The pose track header, or nullptr when the output has none.
inline const Header* PoseHeader(const Output& a_out) {
    const auto* m = a_out.tracks;
    if (!m || m->bytes < 16 || m->bytes > 1024 * 1024 || m->tracks <= kPoseTrack || m->tracks > 128) return nullptr;
    const auto end = sizeof(Master) + static_cast<std::size_t>(m->tracks) * sizeof(Header);
    if (end > static_cast<std::size_t>(m->bytes)) return nullptr;
    return reinterpret_cast<const Header*>(reinterpret_cast<const std::byte*>(m) + sizeof(Master)) + kPoseTrack;
}

// The bone's transform when the pose track is dense (type 1, no flags, stride 48) and holds the bone.
inline float* Bone(const Output& a_out, int a_bone) {
    const auto* h = PoseHeader(a_out);
    if (!h || h->type != 1 || h->flags != 0 || h->stride != 48 || a_bone < 0 || a_bone >= h->count) return nullptr;
    if (h->offset < 0 || static_cast<std::size_t>(h->offset) + static_cast<std::size_t>(h->capacity) * 48 >
                             static_cast<std::size_t>(a_out.tracks->bytes))
        return nullptr;
    return reinterpret_cast<float*>(reinterpret_cast<std::byte*>(a_out.tracks) + h->offset + a_bone * 48);
}

}  // namespace LHEO::PoseTrack
