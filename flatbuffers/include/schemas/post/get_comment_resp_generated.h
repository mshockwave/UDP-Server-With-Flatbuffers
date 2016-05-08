// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_GETCOMMENTRESP_FBS_POST_H_
#define FLATBUFFERS_GENERATED_GETCOMMENTRESP_FBS_POST_H_

#include "flatbuffers/flatbuffers.h"

namespace fbs {
struct Session;
struct GeneralResponse;
}  // namespace fbs

namespace fbs {
namespace post {

struct GetCommentResponse;

struct GetCommentResponse FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  fbs::Status status_code() const { return static_cast<fbs::Status>(GetField<int8_t>(4, 0)); }
  const fbs::Session *session() const { return GetPointer<const fbs::Session *>(6); }
  const flatbuffers::String *content() const { return GetPointer<const flatbuffers::String *>(8); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int8_t>(verifier, 4 /* status_code */) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 6 /* session */) &&
           verifier.VerifyTable(session()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, 8 /* content */) &&
           verifier.Verify(content()) &&
           verifier.EndTable();
  }
};

struct GetCommentResponseBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_status_code(fbs::Status status_code) { fbb_.AddElement<int8_t>(4, static_cast<int8_t>(status_code), 0); }
  void add_session(flatbuffers::Offset<fbs::Session> session) { fbb_.AddOffset(6, session); }
  void add_content(flatbuffers::Offset<flatbuffers::String> content) { fbb_.AddOffset(8, content); }
  GetCommentResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  GetCommentResponseBuilder &operator=(const GetCommentResponseBuilder &);
  flatbuffers::Offset<GetCommentResponse> Finish() {
    auto o = flatbuffers::Offset<GetCommentResponse>(fbb_.EndTable(start_, 3));
    return o;
  }
};

inline flatbuffers::Offset<GetCommentResponse> CreateGetCommentResponse(flatbuffers::FlatBufferBuilder &_fbb,
   fbs::Status status_code = fbs::Status_OK,
   flatbuffers::Offset<fbs::Session> session = 0,
   flatbuffers::Offset<flatbuffers::String> content = 0) {
  GetCommentResponseBuilder builder_(_fbb);
  builder_.add_content(content);
  builder_.add_session(session);
  builder_.add_status_code(status_code);
  return builder_.Finish();
}

inline const fbs::post::GetCommentResponse *GetGetCommentResponse(const void *buf) { return flatbuffers::GetRoot<fbs::post::GetCommentResponse>(buf); }

inline bool VerifyGetCommentResponseBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<fbs::post::GetCommentResponse>(); }

inline void FinishGetCommentResponseBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<fbs::post::GetCommentResponse> root) { fbb.Finish(root); }

}  // namespace post
}  // namespace fbs

#endif  // FLATBUFFERS_GENERATED_GETCOMMENTRESP_FBS_POST_H_
