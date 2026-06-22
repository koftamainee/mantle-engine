// Copyright 2026 Mantle
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

#pragma once

#include <mintload/mmat.h>
#include <mintload/mmesh.h>
#include <mintload/mscb.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include <cstdio>
#include <ktx.h>
#include <ktxvulkan.h>
#include <memory_resource>
#include <string>
#include <vector>

#include "mantle/assets/asset_manager.h"
#include "mantle/assets/types.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/memory/pmr/arena_resource.h"
#include "mantle/core/types.h"
#include "mantle/renderer/gpu_resource_manager.h"
#include "mantle/renderer/renderer.h"

namespace mantle {

    struct LoadedMesh {
        MintMesh mint_mesh {};
        MeshData data {};

        bool loaded = false;
    };

    struct LoadedTexture {
        ImageHandle image;
        u32         bindless_index = UINT32_MAX;
        u32         width = 0;
        u32         height = 0;
        bool        loaded = false;
    };

    struct LoadedMaterial {
        MintMaterial mint_mat {};
        MaterialData data {};
        // Texture uploads (staging buffers + images kept until first frame)
        std::pmr::vector<TextureUpload> texture_uploads {};

        bool loaded = false;
    };

    struct LoadedScene {
        MintScb                                            scb {};
        std::pmr::vector<MeshHandle>                       entity_mesh_handles {};
        std::pmr::vector<std::pmr::vector<MaterialHandle>> entity_material_handles {};
        std::string                                        base_path {};

        bool loaded = false;
    };

    struct AssetManager::Impl {
        Renderer                                  *renderer = nullptr;
        spdlog::logger                            *logger = nullptr;
        std::pmr::polymorphic_allocator<std::byte> alloc {};

        std::pmr::vector<LoadedMesh>     meshes {};
        std::pmr::vector<LoadedMaterial> materials {};
        std::pmr::vector<LoadedScene>    scenes {};

        std::string asset_base_path {"assets"};

        // UUID formatting helpers
        static void uuid_to_str(const uint8_t uuid[16], char *out) {
            static const char hex[17] = "0123456789abcdef";
            for (int i = 0; i < 16; i++) {
                *out++ = hex[uuid[i] >> 4];
                *out++ = hex[uuid[i] & 0xf];
                if (i == 3 || i == 5 || i == 7 || i == 9) {
                    *out++ = '-';
                }
            }
            *out = '\0';
        }

        u32 find_or_load_mesh(const uint8_t uuid[16], const std::string &base) {
            char uuid_str[37];
            uuid_to_str(uuid, uuid_str);

            for (auto &mesh : meshes) {
                if (!mesh.loaded) {
                    continue;
                }
                if (mesh.data.vertex_buffer.is_valid()) {
                }
            }

            u32 idx = static_cast<u32>(meshes.size());
            meshes.push_back({});
            auto &lm = meshes[idx];

            std::string    path = base + "/meshes/" + uuid_str + ".mmesh";
            MintloadResult r = mintload_MmeshLoad(path.c_str(), &lm.mint_mesh);
            if (r != MINTLOAD_SUCCESS) {
                logger->error("Failed to load mesh: {} (err={})", path, static_cast<int>(r));
                return idx;
            }

            usize      vertex_bytes = lm.mint_mesh.vertex_count * 48;
            BufferDesc vb_desc = {
                .size = vertex_bytes,
                .usage = BufferUsage::Vertex,
                .memory = MemoryType::CpuToGpu,
            };
            lm.data.vertex_buffer = renderer->resource_manager().create_buffer(vb_desc, true);
            renderer->resource_manager().update_buffer(lm.data.vertex_buffer,
                                                       lm.mint_mesh.vertex_data, vertex_bytes);

            usize      index_bytes = lm.mint_mesh.index_count * sizeof(u32);
            BufferDesc ib_desc = {
                .size = index_bytes,
                .usage = BufferUsage::Index,
                .memory = MemoryType::CpuToGpu,
            };
            lm.data.index_buffer = renderer->resource_manager().create_buffer(ib_desc, true);
            renderer->resource_manager().update_buffer(lm.data.index_buffer,
                                                       lm.mint_mesh.index_data, index_bytes);

            lm.data.vertex_count = lm.mint_mesh.vertex_count;
            lm.data.index_count = lm.mint_mesh.index_count;

            lm.data.lod_count = lm.mint_mesh.lod_count;
            for (u32 li = 0; li < lm.mint_mesh.lod_count && li < 4; li++) {
                lm.data.lod_first_submesh[li] = lm.mint_mesh.lod_first_submesh[li];
            }

            u32 sm_count = mintload_MmeshSubMeshCount(&lm.mint_mesh);
            lm.data.submeshes.reserve(sm_count);
            for (u32 sm_i = 0; sm_i < sm_count; sm_i++) {
                MintSubMesh msm = mintload_MmeshSubMesh(&lm.mint_mesh, sm_i);
                SubMeshInfo info = {
                    .vertex_count = msm.vertex_count,
                    .vertex_offset = msm.vertex_offset,
                    .vertex_stride = msm.vertex_stride,
                    .index_count = msm.index_count,
                    .index_offset = msm.index_offset,
                    .attr_flags = msm.flags,
                };
                for (int b = 0; b < 3; b++) {
                    info.bounding_min[b] = msm.bounding_min[b];
                    info.bounding_max[b] = msm.bounding_max[b];
                }
                lm.data.submeshes.push_back(info);
            }

            logger->info("Mesh: {} verts, {} idxs, {} sub, {} LODs", lm.mint_mesh.vertex_count,
                         lm.mint_mesh.index_count, lm.mint_mesh.sub_mesh_count,
                         lm.mint_mesh.lod_count);

            lm.loaded = true;
            return idx;
        }

        u32 load_texture(const uint8_t uuid[16], const std::string &base,
                         TextureUpload &out_upload) {
            char uuid_str[37];
            uuid_to_str(uuid, uuid_str);

            std::string path = base + "/textures/" + uuid_str + ".ktx2";

            ktxTexture2 *ktx = nullptr;
            ktxResult    kr = ktxTexture2_CreateFromNamedFile(
                path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx);
            if (kr != KTX_SUCCESS) {
                logger->error("Failed to load texture: {} (err={})", path, static_cast<int>(kr));
                return UINT32_MAX;
            }

            u32 width = static_cast<u32>(ktx->baseWidth);
            u32 height = static_cast<u32>(ktx->baseHeight);
            u32 level_count = static_cast<u32>(ktx->numLevels);

            if (ktxTexture_NeedsTranscoding(ktxTexture(ktx))) {
                KTX_error_code ec = ktxTexture2_TranscodeBasis(ktx, KTX_TTF_RGBA32, 0);
                if (ec != KTX_SUCCESS) {
                    logger->error("Failed to transcode texture: {} (err={})", path,
                                  ktxErrorString(ec));
                    ktxTexture_Destroy(ktxTexture(ktx));
                    return UINT32_MAX;
                }
            }

            ktx_uint8_t *ktx_data = ktxTexture_GetData(ktxTexture(ktx));
            ktx_size_t   ktx_size = ktxTexture_GetDataSize(ktxTexture(ktx));

            VkFormat    vk_fmt = ktxTexture2_GetVkFormat(ktx);
            ImageFormat fmt = ImageFormat::Rgba8Srgb;
            if (vk_fmt == VK_FORMAT_R8G8B8A8_UNORM) {
                fmt = ImageFormat::Rgba8;
            } else if (vk_fmt == VK_FORMAT_R8G8B8A8_SRGB) {
                fmt = ImageFormat::Rgba8Srgb;
            } else if (vk_fmt == VK_FORMAT_B8G8R8A8_SRGB) {
                fmt = ImageFormat::Bgra8Srgb;
            } else if (vk_fmt == VK_FORMAT_R16G16B16A16_SFLOAT) {
                fmt = ImageFormat::Rgba16f;
            } else if (vk_fmt == VK_FORMAT_R32_SFLOAT) {
                fmt = ImageFormat::R32f;
            } else if (vk_fmt == VK_FORMAT_BC7_UNORM_BLOCK) {
                fmt = ImageFormat::Bc7RgbaUnorm;
            } else if (vk_fmt == VK_FORMAT_BC7_SRGB_BLOCK) {
                fmt = ImageFormat::Bc7RgbaSrgb;
            } else if (vk_fmt == VK_FORMAT_BC3_UNORM_BLOCK) {
                fmt = ImageFormat::Bc3RgbaUnorm;
            } else if (vk_fmt == VK_FORMAT_BC3_SRGB_BLOCK) {
                fmt = ImageFormat::Bc3RgbaSrgb;
            } else if (vk_fmt == VK_FORMAT_BC5_UNORM_BLOCK) {
                fmt = ImageFormat::Bc5RgUnorm;
            } else if (vk_fmt == VK_FORMAT_ASTC_4x4_UNORM_BLOCK) {
                fmt = ImageFormat::Astc4x4RgbaUnorm;
            } else if (vk_fmt == VK_FORMAT_ASTC_4x4_SRGB_BLOCK) {
                fmt = ImageFormat::Astc4x4RgbaSrgb;
            }

            ImageDesc img_desc = {
                .width = width,
                .height = height,
                .mip_levels = level_count,
                .format = fmt,
                .usage = ImageUsage::Sampled | ImageUsage::TransferDst,
            };
            auto       &rm = renderer->resource_manager();
            ImageHandle image = rm.create_image(img_desc);
            u32         bindless_idx = rm.get_bindless_index(image, BindlessImageType::Sampled);

            BufferDesc staging_desc = {
                .size = ktx_size,
                .usage = BufferUsage::Transfer,
                .memory = MemoryType::CpuToGpu,
            };
            BufferHandle staging = rm.create_buffer(staging_desc, true);
            rm.update_buffer(staging, ktx_data, ktx_size);

            u32 mip_count = level_count < 16 ? level_count : 16;
            u32 mip_offsets[16] = {};
            for (u32 mi = 0; mi < mip_count; mi++) {
                ktx_size_t offset;
                ktxTexture_GetImageOffset(ktxTexture(ktx), mi, 0, 0, &offset);
                mip_offsets[mi] = static_cast<u32>(offset);
            }

            ktxTexture_Destroy(ktxTexture(ktx));

            out_upload.staging = staging;
            out_upload.image = image;
            out_upload.mip_count = mip_count;
            for (u32 mi = 0; mi < mip_count; mi++) {
                out_upload.mip_offsets[mi] = mip_offsets[mi];
            }

            return bindless_idx;
        }

        u32 find_or_load_material(const uint8_t uuid[16], const std::string &base) {
            char uuid_str[37];
            uuid_to_str(uuid, uuid_str);

            u32 idx = static_cast<u32>(materials.size());
            materials.push_back({});
            auto &lm = materials[idx];

            std::string    path = base + "/materials/" + uuid_str + ".mmat";
            MintloadResult r = mintload_MmatLoad(path.c_str(), &lm.mint_mat);
            if (r != MINTLOAD_SUCCESS) {
                logger->error("Failed to load material: {} (err={})", path, static_cast<int>(r));
                return idx;
            }

            lm.data = {
                .base_color = {lm.mint_mat.base_color[0], lm.mint_mat.base_color[1],
                               lm.mint_mat.base_color[2], lm.mint_mat.base_color[3]},
                .metallic = lm.mint_mat.metallic,
                .roughness = lm.mint_mat.roughness,
                .emissive = {lm.mint_mat.emissive[0], lm.mint_mat.emissive[1],
                             lm.mint_mat.emissive[2]},
                .alpha_cutoff = lm.mint_mat.alpha_cutoff,
                .normal_scale = lm.mint_mat.normal_scale,
                .emissive_strength = lm.mint_mat.emissive_strength,
                .flags = lm.mint_mat.flags,
            };

            if (lm.mint_mat.texture_count > 0) {
                SamplerDesc sampler_desc = {
                    .min_filter = Filter::Linear,
                    .mag_filter = Filter::Linear,
                    .mip_filter = Filter::Linear,
                    .address_u = AddressMode::Repeat,
                    .address_v = AddressMode::Repeat,
                    .max_anisotropy = 1.0f,
                    .min_lod = 0.0f,
                    .max_lod = 16.0f,
                };
                auto         &rm = renderer->resource_manager();
                SamplerHandle sampler = rm.create_sampler(sampler_desc);
                lm.data.bindless_sampler = rm.get_bindless_index(sampler);

                for (u32 ti = 0; ti < lm.mint_mat.texture_count; ti++) {
                    const auto   &slot = lm.mint_mat.textures[ti];
                    TextureUpload upload;
                    u32           bindless_idx = load_texture(slot.texture_guid, base, upload);
                    if (bindless_idx == UINT32_MAX) {
                        continue;
                    }

                    lm.texture_uploads.push_back(upload);

                    switch (slot.type) {
                        case MINTLOAD_TEX_BASECOLOR:
                            lm.data.bindless_basecolor = bindless_idx;
                            break;
                        case MINTLOAD_TEX_NORMAL:
                            lm.data.bindless_normal = bindless_idx;
                            break;
                        case MINTLOAD_TEX_METALLIC_ROUGHNESS:
                            lm.data.bindless_mr = bindless_idx;
                            break;
                        case MINTLOAD_TEX_EMISSIVE:
                            lm.data.bindless_emissive = bindless_idx;
                            break;
                        case MINTLOAD_TEX_OCCLUSION:
                            lm.data.bindless_occlusion = bindless_idx;
                            break;
                    }
                }
            }

            lm.loaded = true;
            return idx;
        }

        BufferHandle build_material_buffer() {
            auto &rm = renderer->resource_manager();
            u32   count = static_cast<u32>(materials.size());
            if (count == 0) {
                return {};
            }

            usize      buf_size = count * sizeof(MaterialGPU);
            BufferDesc desc = {
                .size = buf_size,
                .usage = BufferUsage::Storage,
                .memory = MemoryType::CpuToGpu,
            };
            BufferHandle handle = rm.create_buffer(desc, true);

            std::vector<MaterialGPU> gpu_mats(count);
            for (u32 i = 0; i < count; i++) {
                auto       &m = materials[i].data;
                MaterialGPU mg;
                mg.base_color = {m.base_color[0], m.base_color[1], m.base_color[2],
                                 m.base_color[3]};
                mg.metallic_roughness = {m.metallic, m.roughness, m.emissive_strength, 0.0f};
                mg.emissive_alpha_cutoff = {m.emissive[0], m.emissive[1], m.emissive[2],
                                            m.alpha_cutoff};
                mg.base_color_tex = m.bindless_basecolor;
                mg.normal_tex = m.bindless_normal;
                mg.metallic_roughness_tex = m.bindless_mr;
                mg.emissive_tex = m.bindless_emissive;
                mg.occlusion_tex = m.bindless_occlusion;
                mg.sampler_idx = m.bindless_sampler;
                mg.flags = m.flags;
                gpu_mats[i] = mg;
            }

            rm.update_buffer(handle, gpu_mats.data(), buf_size);
            return handle;
        }
    };

} // namespace mantle
