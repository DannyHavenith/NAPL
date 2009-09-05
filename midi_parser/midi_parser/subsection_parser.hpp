#ifndef SUBSECTION_PARSER_HPP_INCLUDED
#define SUBSECTION_PARSER_HPP_INCLUDED

#include <boost/spirit/home/support/component.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/attribute_of.hpp>
#include <boost/spirit/home/qi/domain.hpp>
#include <boost/spirit/home/qi/skip.hpp>
#include <boost/spirit/home/qi/meta_grammar.hpp>
#include <boost/proto/traits.hpp>


#include <iterator> // for std::advance

using namespace std;

namespace local_spirit_components
{


    //
    // Proto tags.
    //
    namespace tag
    {
            struct subsection{};
    }


    using namespace ::boost::spirit;

    //
    // Spirit components.
    //
    typedef ::boost::proto::terminal< tag::subsection>::type subsection_type;


    const subsection_type subsection = {{}};

    // the director
    struct subsection_director
    {
        template <typename Component, typename Context, typename Iterator>
        struct attribute
        {
            typedef typename
                result_of::subject<Component>::type
            subject_type;

            typedef typename
                traits::attribute_of<
                    qi::domain, subject_type, Context, Iterator>::type
            type;
        };


        template <
            typename Component
          , typename Iterator, typename Context
          , typename Skipper, typename Attribute>
        static bool parse(
            Component const& component
          , Iterator& first, Iterator const& last
          , Context& context, Skipper const& skipper
          , Attribute& attr)
        {
            typedef typename
                result_of::subject<Component>::type::director
            director;

            // limit parsing to the range (first,first+arg1>
            qi::skip(first, last, skipper);
            Iterator local_end = first;
//            std::advance( first, boost::proto::arg_c<0>(argument1(component)));
            size_t offset = boost::proto::child_c<0>(argument1(component))(unused, context);
            std::advance( local_end, offset);
            return director::parse(
                subject(component),
                first,  local_end,
                context, skipper, attr);
        }

        template <typename Component, typename Context>
        static std::string what(Component const& component, Context const& ctx)
        {
            std::string result = "subsection[";

            typedef typename
                result_of::subject<Component>::type::director
            director;

            result += director::what(subject(component), ctx);
            result += "]";
            return result;
        }
    };

    struct subsection_meta_grammar
        : meta_grammar::subscript_function1_rule< // subsection(val)[p]
            qi::domain, tag::subsection,
            subsection_director, ::boost::proto::_, qi::main_meta_grammar>
    {
    };

//        int x = subsection_meta_grammar();
}

using namespace local_spirit_components;

namespace boost { namespace spirit { namespace qi {

    // hook up the subsection meta grammar into the spirit meta grammar.
    template< typename TExpr >
    struct is_valid_expr<
        TExpr,
        typename ::boost::enable_if< ::boost::proto::matches< TExpr, subsection_meta_grammar > >::type
    >
    : ::boost::mpl::true_
    {};

    template< typename TExpr >
    struct expr_transform<
        TExpr,
        typename ::boost::enable_if< ::boost::proto::matches< TExpr, subsection_meta_grammar > >::type
    >
    : ::boost::mpl::identity< subsection_meta_grammar >
    {};
}}}
#endif // SUBSECTION_PARSER_HPP_INCLUDED
