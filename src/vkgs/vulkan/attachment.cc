#include "vkgs/vulkan/attachment.h"

#include "vk_mem_alloc.h"

#include "vkgs/vulkan/context.h"

namespace vkgs {
namespace vk {

class Attachment::Impl {
 public:
  Impl(Context context, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits samples,
       bool input_attachment)
      : context_(context), width_(width), height_(height), format_(format) {
    usage_ = 0;
    VkImageAspectFlags aspect = 0;
    switch (format) {
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
        usage_ = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;

      default:
        usage_ = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        break;
    }

    if (input_attachment) {
      usage_ |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    VkImageCreateInfo image_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = format;
    image_info.extent = {width_, height_, 1};
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = samples;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = usage_;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo alloc_info = {};
    alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    vmaCreateImage(context.allocator(), &image_info, &alloc_info, &image_, &allocation_, NULL);

    VkImageViewCreateInfo image_view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    image_view_info.image = image_;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = format;
    image_view_info.subresourceRange = {aspect, 0, 1, 0, 1};
    vkCreateImageView(context.device(), &image_view_info, NULL, &image_view_);
  }

  ~Impl() {
    vkDestroyImageView(context_.device(), image_view_, NULL);
    vmaDestroyImage(context_.allocator(), image_, allocation_);
  }

  operator VkImageView() const noexcept { return image_view_; }

  VkImage image() const noexcept { return image_; }
  VkImageUsageFlags usage() const noexcept { return usage_; }
  VkFormat format() const noexcept { return format_; }
  ImageSpec image_spec() const noexcept { return ImageSpec{width_, height_, usage_, format_}; }

 private:
  Context context_;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  VkImageUsageFlags usage_ = 0;
  VkFormat format_ = VK_FORMAT_UNDEFINED;
  VkImage image_ = VK_NULL_HANDLE;
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VkImageView image_view_ = VK_NULL_HANDLE;
};

Attachment::Attachment() = default;

Attachment::Attachment(Context context, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits samples,
                       bool input_attachment)
    : impl_(std::make_shared<Impl>(context, width, height, format, samples, input_attachment)) {}

Attachment::~Attachment() = default;

Attachment::operator VkImageView() const { return *impl_; }

VkImage Attachment::image() const { return impl_->image(); }

VkImageUsageFlags Attachment::usage() const { return impl_->usage(); }

VkFormat Attachment::format() const { return impl_->format(); }

ImageSpec Attachment::image_spec() const { return impl_->image_spec(); }

}  // namespace vk
}  // namespace vkgs
