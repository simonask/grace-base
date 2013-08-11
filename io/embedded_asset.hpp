//
//  embedded_asset.hpp
//  grace
//
//  Created by Simon Ask Ulsnes on 25/06/13.
//  Copyright (c) 2013 simonask. All rights reserved.
//

#ifndef grace_embedded_asset_hpp
#define grace_embedded_asset_hpp

namespace grace {
	struct EmbeddedAsset {
		const byte* data;
		size_t      size;
		const char* rid;
	};
}

#endif
