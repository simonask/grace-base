//
//  yaml_archive.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 27/07/12.
//  Copyright (c) 2012 Simon Ask Consulting. All rights reserved.
//

#ifndef grace_yaml_archive_hpp
#define grace_yaml_archive_hpp

#include "serialization/archive.hpp"

namespace grace {
	struct YAMLArchive : public Archive {
		explicit YAMLArchive(IAllocator& alloc = default_allocator()) : Archive(alloc) {}
		void write(OutputStream& os) const override;
		size_t read(InputStream& is, String& out_error) override;
		bool can_parse(const byte* begin, const byte* end) const;
	};
}

#endif
